/*******************************************************
 Windows HID simplification
 Alan Ott
 Signal 11 Software
 8/22/2009
 Copyright 2009
 This contents of this file may be used by anyone
 for any reason without any conditions and may be
 used as a starting point for your own applications
 which use HIDAPI.
********************************************************/

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#define MAX_STR 255
#define FAIL_USB -100

unsigned char usb_data[33];
const uint16_t logitech_vendor_id = 0x046d;
const uint16_t logitech_device_g203 = 0xc084;

int
handleUSB()
{
  int res;
	unsigned char buf[256];
	wchar_t wstr[MAX_STR];
	hid_device *handle;
	int i;

	struct hid_device_info *devs, *cur_dev;

	if (hid_init())
		return -1;

	devs = hid_enumerate(logitech_vendor_id, logitech_device_g203);
	cur_dev = devs;
	if (cur_dev)
  {
		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		printf("\n");
		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n", cur_dev->product_string);
		printf("  Release:      %hx\n", cur_dev->release_number);
		printf("  Interface:    %d\n",  cur_dev->interface_number);
		printf("  Usage (page): 0x%hx (0x%hx)\n", cur_dev->usage, cur_dev->usage_page);
		printf("\n");
		cur_dev = cur_dev->next;
	}
  else
  {
   fprintf(stderr, "Could not get device descriptor\n");
   return FAIL_USB;
  }
	hid_free_enumeration(devs);

	// Set up the command buffer.
	memset(buf,0x00,sizeof(buf));
	buf[0] = 0x01;
	buf[1] = 0x81;


	// Open the device using the VID, PID,
	// and optionally the Serial number.
	////handle = hid_open(0x4d8, 0x3f, L"12345");
	handle = hid_open(logitech_vendor_id, logitech_device_g203, NULL);
	if (!handle) {
		printf("unable to open device\n");
 		return FAIL_USB;
	}

  /*
	// Send a Feature Report to the device
  memset(&usb_data, '\0', sizeof(usb_data));
  //memcpy(&usb_data, "\x10\xff\x0d\x2e\x01\x00\x00", 7);
  memcpy(&usb_data, "\x11\xff\x0e\x3e\x00\x01\xfd\x76\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 20);
	res = hid_send_feature_report(handle, usb_data, 17);
	if (res < 0) {
		printf("Unable to send a feature report.\n");
	}

	// Read a Feature Report from the device
	buf[0] = 0x2;
	res = hid_get_feature_report(handle, buf, sizeof(buf));
	if (res < 0) {
		printf("Unable to get a feature report.\n");
		printf("%ls", hid_error(handle));
	}
	else {
		// Print out the returned buffer.
		printf("Feature Report\n   ");
		for (i = 0; i < res; i++)
			printf("%02hhx ", buf[i]);
		printf("\n");
	}
  */

	// Toggle LED (cmd 0x80). The first byte is the report number (0x1).
  memset(&usb_data, '\0', sizeof(usb_data));
  memcpy(&usb_data, "\x11\xff\x0e\x3e\x00\x01\xfd\x76\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x21", 21);
	res = hid_write(handle, usb_data, 21);
	if (res < 0) {
		printf("Unable to write()\n");
		printf("Error: %ls\n", hid_error(handle));
	}


  /*
	// Request state (cmd 0x81). The first byte is the report number (0x1).
	buf[0] = 0x1;
	buf[1] = 0x81;
	hid_write(handle, buf, 17);
	if (res < 0)
		printf("Unable to write() (2)\n");
  */

	hid_close(handle);

	/* Free static HIDAPI objects. */
	hid_exit();

  return 0;
}

int
main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

  printf("hidapi test/example tool. Compiled with hidapi version %s, runtime version %s.\n", HID_API_VERSION_STR, hid_version_str());
	if (hid_version()->major == HID_API_VERSION_MAJOR && hid_version()->minor == HID_API_VERSION_MINOR && hid_version()->patch == HID_API_VERSION_PATCH) {
		printf("Compile-time version matches runtime version of hidapi.\n\n");
	}
	else {
		printf("Compile-time version is different than runtime version of hidapi.\n]n");
	}
  int output = handleUSB();
  printf("%d\n", output);
	#ifdef WIN32
	system("pause");
#endif

	return 0;
}
