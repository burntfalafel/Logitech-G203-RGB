//// @file
//// @brief Uses the native libusb library to interact with the G203, which is not setting up a correct HID handshake
////
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#include "libusb-1.0/libusb.h"

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

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

uint8_t MESSAGE_SET_CONFIGURATION[URB_MESSAGE_LENGTH] = {};
const uint16_t logitech_vendor_id = 0x046d;
const uint16_t logitech_device_g203 = 0xc084;
static void print_devs(libusb_device **devs)
{
	libusb_device *dev;
	int i = 0, j = 0;
	uint8_t path[8];

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			fprintf(stderr, "failed to get device descriptor");
			return;
		}

		printf("%04x:%04x (bus %d, device %d)",
			desc.idVendor, desc.idProduct,
			libusb_get_bus_number(dev), libusb_get_device_address(dev));

		r = libusb_get_port_numbers(dev, path, sizeof(path));
		if (r > 0) {
			printf(" path: %d", path[0]);
			for (j = 1; j < r; j++)
				printf(".%d", path[j]);
		}
		printf("\n");
	}
}

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
    msleep(20);
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
	struct libusb_device_descriptor desc;
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

  retVal = libusb_claim_interface(retHandle, dInterfaceNumber);
  if(retVal<0)
  {
    fprintf(stderr, "Could not claim interface: %s.\n", libusb_error_name(retVal));
    libusb_free_config_descriptor(dConfig);
    libusb_close(retHandle);
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    return retVal;
  }
  printf("Claimed interface %d \n", dInterfaceNumber);

  for (uint32_t i=0; i<numMessages; i++)
    for (uint32_t j=0; j<uMessages[i].iterations; j++)
      controlTransfer(retHandle, uMessages[i].wValue, uMessages[i].usb_data, uMessages[i].wLength);
    
  printf("Cleanup\n");
  libusb_release_interface(retHandle, dInterfaceNumber);
  libusb_free_config_descriptor(dConfig);
  libusb_close(retHandle);
  libusb_free_device_list(devs, 1);
  libusb_exit(NULL);
  return 0;
}

int main(void)
{
  usbMessages colorMessages[] = {
    { 1, 0x0210, (unsigned char*)"\x10\xff\x0f\x2f\x00\x00\x00",  7},
    { 35, 0x0210, (unsigned char*)"\x10\xff\x0f\x0f\x00\x00\x00",  7},
    { 1, 0x0211, (unsigned char*)"\x11\xff\x0f\x6f\x00\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 20},
    { 1, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\x01\x01\x00\x78\x05\xf6\x09\x7a\x0d\xfe\x10\x56\x13\xff\xff\xff", 20},
    { 1, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\xff\x00\x15\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 20},
    { 1, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\x80\x01\x00\x01\x80\x01\x00\x02\x80\x01\x00\x04\x80\x01\x00\x08", 20},
    { 1, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\x80\x01\x00\x10\x90\x05\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 20},
    { 2, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 20},
    { 1, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\x80\x01\x00\x01\x80\x01\x00\x02\x80\x01\x00\x04\x80\x01\x00\x08", 20},
    { 1, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\x80\x01\x00\x10\x90\x05\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 20},
    { 2, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 20},
    { 1, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\x50\x00\x72\x00\x6f\x00\x66\x00\x69\x00\x6c\x00\x65\x00\x20\x00", 20},
    { 1, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 20},
    { 1, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 20},
    { 1, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\x01\xbf\x00\xff\x02\x00\x00\x00\x00\x00\x00\x01\xbf\x00\xff\x02", 20},
    { 1, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 20},
    { 1, 0x0211, (unsigned char*)"\x11\xff\x0f\x7f\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x05\xed", 20},
    { 1, 0x0210, (unsigned char*)"\x10\xff\x0f\x8f\x00\x00\x00", 7}
  };
  int status = handlerUSB(colorMessages, 18);
  printf("%d\n",status);
	return 0;
}