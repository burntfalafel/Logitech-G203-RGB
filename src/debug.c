#include<stdio.h>

void printUsbData (unsigned char *sData, uint16_t wLength){
    for (uint16_t i=0; i<=wLength; i++)
      printf("%x",sData[i]);
}

void printdConfigInterfaces(struct libusb_config_descriptor *dConfig) {
  for (int i =0; i<10;i++)
      printf("%0.2x\n", dConfig->interface[0].altsetting[i].bInterfaceNumber);
    printf("---------------------------\n");
  for (int i =0; i<10;i++)
      printf("%0.2x\n", dConfig->interface[0].altsetting[i].bInterfaceClass);
    printf("---------------------------\n");
  for (int i =0; i<10;i++)
      printf("%0.2x\n", dConfig->interface[0].altsetting[i].bLength);
    printf("---------------------------\n");
}

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
