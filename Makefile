HID_CFILES = src/G203.c
LIBUSB_CFILES = src/G203-native-libusb.c # means orginal
CFILES = $(HID_CFILES) $(LIBUSB_CFILES)
IDIR = -Iinclude/ -I/usr/local/include/hidapi
CC = gcc
CFLAGS= -std=gnu11 -Wall -Wextra $(IDIR)

LDIR = -L/usr/local/lib  -L/usr/lib/x86_64-linux-gnu

LIBS = -lusb-1.0 -lpthread -Wl,-rpath -Wl,/usr/local/lib -lhidapi-libusb #with HID

TARGET = G203

all: $(TARGET)

$(TARGET): $(CFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(CFILES) $(LDIR) $(LIBS)

debug: $(LIBUSB_CFILES)
	$(CC) $(CFLAGS) -g -o $(TARGET) $(LIBUSB_CFILES) $(LDIR) $(LIBS)

hid_debug: $(HID_CFILES)
	$(CC) $(CFLAGS) -g -o $(TARGET) $(HID_CFILES) $(LDIR) $(LIBS)

clean:
	$(RM) $(TARGET)
