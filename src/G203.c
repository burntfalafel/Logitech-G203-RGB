#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <libusb-1.0/libusb.h>

const uint16_t logitech_vendor_id = 0x046d;
const uint16_t logitech_device_g203 = 0xc084;

// discover devices
libusb_device **list;
libusb_device *found = NULL;
ssize_t cnt = libusb_get_device_list(NULL, &list);
ssize_t i = 0;
int err = 0;
if (cnt < 0)
    error();
 
for (i = 0; i < cnt; i++) {
    libusb_device *device = list[i];
    if (is_interesting(device)) {
        found = device;
        break;
    }
}
 
if (found) {
    libusb_device_handle *handle;
 
    err = libusb_open(found, &handle);
    if (err)
        error();
    // etc
}
 
libusb_free_device_list(list, 1);
