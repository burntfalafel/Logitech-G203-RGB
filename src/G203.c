#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <libusb-1.0/libusb.h>

const uint16_t logitech_vendor_id = 0x046d;
const uint16_t logitech_device_g203 = 0xc084;

int checkDevice(
