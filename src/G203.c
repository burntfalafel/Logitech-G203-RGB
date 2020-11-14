#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "libusb-1.0/libusb.h"
#define FAIL_USB 0;

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

int checkDevice(libusb_device *pDevice)
{
    struct libusb_device_descriptor devDesc;
    libusb_get_device_descriptor(pDevice, &devDesc);
    printf("Checking device %04x:%04x, address %d\n",
             devDesc.idVendor, devDesc.idProduct,
             libusb_get_device_address(pDevice));
    if(devDesc.idVendor == logitech_vendor_id)
    {
      if (devDesc.idProduct == logitech_vendor_id)
        return 1;
    }
    return 0;
}

int handlerUSB()
{
  int retVal = 0;
	struct libusb_device_descriptor desc;
	libusb_device **devs = 0;
  libusb_device *deviceLogitech = 0;
  libusb_device_handle *retHandle = 0;
  ssize_t cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0){
		libusb_exit(NULL);
    fprintf(stderr, "Could not fetch USB device list.\n");
		return (int) cnt;
	}
  for (ssize_t i = 0; i <= cnt; i++)
  {
    if(checkDevice(devs[i]))
      deviceLogitech = devs[i];
  }
  if(!deviceLogitech)
  {
    fprintf(stderr, "Could not find inquired device from USB device list\n");
    return FAIL_USB;
  }
  retVal = libusb_open(deviceLogitech, &retHandle);
  if(retVal<0)
  {
    fprintf(stderr, "Could not open inquired device");
    return FAIL_USB;
  }


}

int main(void)
{
	int r;
	ssize_t cnt;

	r = libusb_init(NULL);
	if (r < 0)
		return r;



	print_devs(devs);
	libusb_free_device_list(devs, 1);

	libusb_exit(NULL);
	return 0;
}
