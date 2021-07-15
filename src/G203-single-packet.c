//// @file
//// @brief Uses the native libusb library to interact with the G203, which is not setting up a correct HID handshake
////
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>

#include "libusb-1.0/libusb.h"
#include "gopt/gopt.h"

#define FAIL_USB -100
#define URB_MESSAGE_LENGTH 28
#define SETUP_CONFIG_MESSAGE_LENGTH 8
#define SETUP_SYNC_MESSAGE_LENGTH 15
#define SETUP_COLORS_MESSAGE_LENGTH 28
#define MAX_NUM_COLORS 4
#define MAX_NUM_SCALARS 4

typedef struct {
  uint16_t nDpi;
} Dpi;

typedef struct {
    uint8_t nRed;
    uint8_t nGreen;
    uint8_t nBlue;
} Color;

typedef struct {
  uint32_t iterations;
  uint32_t wValue;
  unsigned char* usb_data;
  uint16_t wLength;
} usbMessages;

typedef struct {
    Color colors[MAX_NUM_COLORS];
    int scalars[MAX_NUM_SCALARS];
} Arguments;

uint8_t MESSAGE_SET_CONFIGURATION[URB_MESSAGE_LENGTH] = {};
const uint16_t logitech_vendor_id = 0x046d;
const uint16_t logitech_device_g203 = 0xc084;

int
controlTransfer(libusb_device_handle *pHandle, uint32_t wValue, unsigned char *sData, uint16_t wLength) {
    int retVal = libusb_control_transfer(
        pHandle,
        0x21 /* bmRequestType */,
        0x09 /* bRequest */,
        wValue /* wValue */,
        0x0001 /* wIndex */,
        sData,
        wLength,
        0 /* standard device timeout */
        );
    if (retVal < 0) {
        fprintf(stderr, "Control transfer error: %s\n", libusb_error_name(retVal));
    }
    return retVal;
}

int
checkDevice(libusb_device *pDevice)
{
    struct libusb_device_descriptor devDesc;
    int nRetVal;
    nRetVal = libusb_get_device_descriptor(pDevice, &devDesc);
    if(nRetVal<0)
      fprintf(stderr, "Could not get device descriptor\n");
    printf("Checking device %04x:%04x, address %d\n",
             devDesc.idVendor, devDesc.idProduct,
             libusb_get_device_address(pDevice));
    if(devDesc.idVendor == logitech_vendor_id)
    {
      if (devDesc.idProduct == logitech_device_g203)
      {
        printf("Found Logitech G203 Device descriptor \n");
        return 1;
      }
    }
    return 0;
}

int
handlerUSB( usbMessages *uMessages, uint32_t numMessages )
{
  int retVal = 0;
	libusb_device **devs = 0;
  libusb_device *deviceLogitech = 0;
  libusb_device_handle *retHandle = 0;
  struct libusb_config_descriptor *dConfig;
  uint8_t dInterfaceNumber = 0;
	int r = libusb_init(NULL);
	if (r < 0)
		return r;
  ssize_t cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0){
		libusb_exit(NULL);
    fprintf(stderr, "Could not fetch USB device list.\n");
		return (int) cnt;
	}
  for (ssize_t i = 0; i < cnt; i++)
  {
    if(checkDevice(devs[i]))
    {
      deviceLogitech = devs[i];
      break;  //remove this break statement if logitech_device_g203 is an array
    }
  }
  if(!deviceLogitech)
  {
    fprintf(stderr, "Could not find inquired device from USB device list\n");
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    return FAIL_USB;
  }
  retVal = libusb_open(deviceLogitech, &retHandle);
  if(retVal<0)
  {
    fprintf(stderr, "Could not open inquired device\n");
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    return (int) retVal;
  }
  printf("Inquired device open \n");
  retVal = libusb_set_auto_detach_kernel_driver(retHandle,1);
    if (retVal < 0) {
        fprintf(stderr, "Could not set auto detach kernel mode: %s\n",
                libusb_error_name(retVal));
    libusb_close(retHandle);
    libusb_exit(NULL);
    return (int) retVal;
    } else {
        printf("Auto detach kernel mode set.\n");
    }
  retVal = libusb_get_active_config_descriptor(deviceLogitech, &dConfig);
  if(retVal<0)
  {
    fprintf(stderr, "Could not get configuration descriptor: %s.\n", libusb_error_name(retVal));
    libusb_close(retHandle);
    libusb_exit(NULL);
    return (int) retVal;
  }
  printf("Got configuration descriptor \n");
  if (dConfig->bNumInterfaces == 0)
  {
    fprintf(stderr, "No interafces defined on USB device\n");
    libusb_free_config_descriptor(dConfig);
    libusb_close(retHandle);
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    return FAIL_USB;
  }
  printf("Found %.2x interfaces on the USB device\n", dConfig->bNumInterfaces);

  printf("With %d number of settings\n", dConfig->interface[2].num_altsetting);

  if(dConfig->interface[1].num_altsetting == 0)
  {
    fprintf(stderr, "No interface descriptors for the first interface of the USB device.\n");
    libusb_free_config_descriptor(dConfig);
    libusb_close(retHandle);
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    return FAIL_USB;
  }

  dInterfaceNumber = dConfig->interface[1].altsetting[0].bInterfaceNumber;

  for (uint32_t i=0; i<numMessages; i++)
    for (uint32_t j=0; j<uMessages[i].iterations; j++)
    {
    retVal = libusb_claim_interface(retHandle, dInterfaceNumber); // this should not be in for loop - for now fixes the libusb library bug
    if(retVal<0)
    {
      fprintf(stderr, "Could not claim interface: %s.\n", libusb_error_name(retVal));
      goto cleanup;
    }
    controlTransfer(retHandle, uMessages[i].wValue, uMessages[i].usb_data, uMessages[i].wLength);
    libusb_release_interface(retHandle, dInterfaceNumber); // this should not be in for loop - for now fixes the libusb library bug
    }
  cleanup:
  printf("Cleanup\n");
  libusb_free_config_descriptor(dConfig);
  libusb_close(retHandle);
  libusb_free_device_list(devs, 1);
  libusb_exit(NULL);
  return retVal;
}

int formatColor(char* color, unsigned char color_fmt[3]) {
  int mac_byte;
  unsigned char byte;
  for(int i=0; i< 6; i++)
  {
    if((color[i]<'A' || color[i]>'z') && (color[i]<'0' || color[i]>'9'))
    {
      fprintf(stderr, "Color supplied is not Hex");
      return 1;
    }
  }
 for (int i = 0; i<3; i++)
 {
    sscanf(color,"%x",&mac_byte);
    byte = (mac_byte>>8*i) & 0xFF;
    color_fmt[2-i] = byte;
 }
 return 0;
}
unsigned char* 
colorString( char color[6])
{
  unsigned char color_fmt[3];
  unsigned char *usb_data = (unsigned char*) malloc(sizeof(unsigned char)*20);
  memset(usb_data, '\0', 20);
  memcpy(usb_data, "\x11\xff\x0e\x3b\x00\x01", 6);
  if(formatColor(color, color_fmt))
  {
    return NULL;
  }
  memcpy(usb_data+6, color_fmt, 3);
  memcpy(usb_data+6+sizeof(color_fmt), "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 11);
  return usb_data;
}

int 
solidColor(char* color) {
  int status = 0;
  unsigned char *colorData = colorString(color);
  if (colorData){
    usbMessages colorMessages[] = {
      { 1, 0x0211, colorData , 20}, 
    };
    status = handlerUSB(colorMessages, 1);
  }
  else
  {
    fprintf(stderr, "Failed to initialize colorString()\n");
    status = 1;
  }
  free(colorData);
  return status;
}

int converthex(char x)
{
  return strtoul(&x,NULL,16);
}

int digits_only(const char *s)
{
    while (*s) {
        if (isdigit(*s++) == 0) return 1;
    }

    return 0;
}

int parseStringToInt(char* string, int *params, int len)
{
  const char *delim = ",";
  char *t = strtok(string, delim);
  int i = 0;
  while (t!=NULL){
    if(!digits_only(t))
      params[i] = strtol(t,NULL,10);
    t = strtok(NULL, delim);
    i++;
    if (i>len)
    {
      fprintf(stderr, "Excess number of argments supplied\n");
      return 1;
    }
  }
  if(i != len) {
    fprintf(stderr, "Wrong type of argumnets supplied\n");
    return 1;
  }
  return 0;
}

unsigned char*
cycleColorString(int rate, int bright){
  if (rate<1000 || rate>60000) {
    fprintf(stderr, "Rate value parameter exceeds limit ");
    return NULL;
  }
  if (bright<1 || bright>100) {
    fprintf(stderr, "Brightness parameter exceeds limit ");
    return NULL;
  }
  unsigned char *usb_data = (unsigned char*) malloc(sizeof(unsigned char)*20);
  memset(usb_data, '\0', 20);
  memcpy(usb_data, "\x11\xff\x0e\x3b\x00\x02\x00\x00\x00\x00\x00", 11);
  unsigned char brate[2];
  brate[0] = (rate>>8) & 0xFF;
  brate[1] = rate & 0xFF;
  memcpy(usb_data+11, brate, 2);
  unsigned char bbright = bright & 0xFF;
  memcpy(usb_data+11+2, &bbright, 1);
  memcpy(usb_data+11+2+1, "\x00\x00\x00\x00\x00\x00", 6);
  return usb_data;
  }

int 
cycleColor(char* params) {
  int intparams[2];
  int status = parseStringToInt(params, intparams, 2);
  if (status)
  {
    fprintf(stderr, "parseStringToInt failed");
    return status;
  }
  unsigned char* colorData = cycleColorString(intparams[0], intparams[1]);
  if(colorData){
    usbMessages colorMessages[] = {
        { 1, 0x0211, colorData , 20},
      };
    status = handlerUSB(colorMessages, 1);
  free(colorData);
  }
  else {
    fprintf(stderr, "Failed to initalise cycleColorString()");
    status = -1;
  }
  return status;
}

unsigned char*
breatheColorString(char* color, int rate, int bright){
  if (rate<1000 || rate>60000) {
    fprintf(stderr, "Rate value parameter exceeds limit\n");
    return NULL;
  }
  if (bright<1 || bright>100) {
    fprintf(stderr, "Brightness parameter exceeds limit\n");
    return NULL;
  }
  unsigned char color_fmt[3];
  unsigned char *usb_data = (unsigned char*) malloc(sizeof(unsigned char)*20);
  memset(usb_data, '\0', 20);
  
  if(formatColor(color, color_fmt))
  {
    return NULL;
  }
  memcpy(usb_data, "\x11\xff\x0e\x3b\x00\x03", 6);
  memcpy(usb_data+6, color_fmt, 3);
  unsigned char brate[2];
  brate[0] = (rate>>8) & 0xFF;
  brate[1] = rate & 0xFF;
  memcpy(usb_data+9, brate, 2);
  memcpy(usb_data+9+2, "\x00", 1);
  unsigned char bbright = bright & 0xFF;
  memcpy(usb_data+9+1+2, &bbright, 1);
  memcpy(usb_data+9+1+2+1, "\x00\x00\x00\x00\x00\x00\x00", 7);
  return usb_data;
}


int 
breatheColor(char* params) {
  int intparams[3];
  
  int status = parseStringToInt(params, intparams, 3);
  if (status)
  {
    fprintf(stderr, "parseStringToInt failed\n");
    return status;
  }
  const char *delim = ",";
  char *params_prfx = strtok(params, delim);
  
  unsigned char* colorData = breatheColorString(params_prfx, intparams[1], intparams[2]);
  if(colorData){
    usbMessages colorMessages[] = {
        { 1, 0x0211, colorData , 20},
      };
    status = handlerUSB(colorMessages, 1);
  free(colorData);
  }
  else {
    fprintf(stderr, "Failed to initalise breatheColorString()\n");
    status = -1;
  }
  return status;
}

unsigned char* 
dpiString( int dpi)
{
  if(dpi<200 || dpi>8000)
  {
    fprintf(stderr, "DPI value parameter exceeds limit");
    return NULL; 
  }
  unsigned char *usb_data = (unsigned char*) malloc(sizeof(unsigned char)*7);
  memset(usb_data, '\0', 7);
  memcpy(usb_data, "\x10\xff\x0a\x3b\x00", 5);
  unsigned char bdpi[2];
  bdpi[0] = (dpi>>8) & 0xFF;
  bdpi[1] = dpi & 0xFF;
  memcpy(usb_data+5, bdpi, 2);
  return usb_data;
}

int 
changeDPI(char* params) {
  int dpiValue = strtol(params,NULL, 10);
  int status = 0;
  unsigned char *dpiData = dpiString(dpiValue);
  if (dpiData){
    usbMessages dpiMessages[] = {
      { 1, 0x0210, dpiData,  7},
    };
    status = handlerUSB(dpiMessages, 1);
  free(dpiData);
  }
  else {
    fprintf(stderr, "Failed to initalise dpiString()");
    status = -1;
  }
  return status;
}

void remove_spaces(char* s) {
    const char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

int 
main(int argc, const char **argv)
{
  int status = 1;

  const char* cparams;
  void *options= gopt_sort( & argc, argv, gopt_start(
      gopt_option( 'h', 0, gopt_shorts( 'h', '?' ), gopt_longs( "help", "HELP" )),
      gopt_option( 'c', GOPT_ARG, gopt_shorts( 'c' ), gopt_longs( "cycle", "CYCLE" )),
      gopt_option('b', GOPT_ARG, gopt_shorts('b'), gopt_longs("breathe", "BREATH")),
      gopt_option( 'o', 0, gopt_shorts( 'o' ), gopt_longs( "off", "OFF" )),
      gopt_option( 's', GOPT_ARG, gopt_shorts( 's' ), gopt_longs( "solid", "SOLID" )),
      gopt_option( 'd', GOPT_ARG, gopt_shorts( 'd' ), gopt_longs( "dpi", "DPI" ))));
  
  if(gopt(options, 'h')) {
    printf("\nLogitech G203 Prodigy mouse LED control \n\n Arguments \n \
    -h, --help                                      Show this help message and exit\n \
    -c, --cycle <rate>,<brightness>                 Cycles the RGB color for the mouse\n \
    -b. --breathe <color>,<rate>,<brightness>       Breathes a RGB color with rate a brightness\n \
    -o, --off                                       Switches off the RGB color for the mouse \n \
    -s, --solid <color>                             Solid color used for the mouse - has to be hex \n \
    -d, --dpi                                       dpi value to be set for the mouse - has to be decimal\n \
    \n \n\
    Limits: \n\
    \t\tcolor:RRGGBB\n\t\trate:1000-60000(number of milliseconds)\n\t\tbrightness:1-100\
    \n\n\
    Examples:\n\
    \t\tG203colorshifter -c 10000,100\n\t\tG203colorshifter -b FF000,10000,100\n");
  }
  if (gopt_arg(options, 'c', &cparams)) {
    char* params = strdup(cparams);
    remove_spaces(params);
    status = cycleColor(params);
    return status;
  }
  if (gopt_arg(options, 'b', &cparams)) {
    char* params = strdup(cparams);
    remove_spaces(params);
    status = breatheColor(params);
    return status;
  }
  if (gopt(options, 'o')) {
    status = solidColor("000000");
    return status;
  }
  if (gopt_arg(options, 'd', &cparams)) {
    char* params = strdup(cparams);
    remove_spaces(params);
    status = changeDPI(params);
    return status;
  }
  if ( gopt_arg(options, 's', &cparams) && strcmp (cparams, "-")) {
    char* params = strdup(cparams);
    remove_spaces(params);
    status = solidColor(params);
    return status;
  }
  
  return status;
}