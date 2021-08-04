CFILES = include/gopt/gopt.c src/G203-single-packet.c
MMO_CFILES = include/gopt/gopt.c src/G203-native-libusb.c # means orginal
IDIR = -Iinclude/ -I/usr/local/include/hidapi
CC = clang
CFLAGS= -std=gnu11 -Wall -Wextra $(IDIR)

LDIR = -L/usr/local/lib  -L/usr/lib/x86_64-linux-gnu
LIBS = -lusb-1.0 -lpthread -Wl,-rpath -Wl,/usr/local/lib #with HID

TARGET = G203colorshifter
MMO_TARGET = G203MMO
DESTDIR = /usr/local

all: $(TARGET) $(MMO_TARGET)

$(TARGET): $(CFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(CFILES) $(LDIR) $(LIBS)

$(MMO_TARGET): $(MMO_CFILES)
	$(CC) $(CFLAGS) -o $(MMO_TARGET) $(MMO_CFILES) $(LDIR) $(LIBS)

debug: $(MMO_CFILES)
	$(CC) $(CFLAGS) -g -o $(TARGET) $(MMO_CFILES) $(LDIR) $(LIBS)

single_debug: $(CFILES)
	$(CC) $(CFLAGS) -g -o $(TARGET) $(CFILES) $(LDIR) $(LIBS)

install:
	install -d $(DESTDIR)/bin/
	install -m 755 $(TARGET) $(DESTDIR)/bin/
ifneq ("$(wildcard $(MMO_TARGET))","")
	install -m 755 $(MMO_TARGET) $(DESTDIR)/bin/
endif

.PHONY: install clean # check if bins are present

uninstall:
	rm -rf $(DESTDIR)/bin/$(TARGET)
	rm -rf $(DESTDIR)/bin/$(MMO_TARGET)

clean:
	$(RM) $(TARGET) $(MMO_TARGET)
