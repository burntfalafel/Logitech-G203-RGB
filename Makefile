CFILES = src/G203.c
IDIR = -Iinclude/ -I/usr/local/include/hidapi
CC = gcc
CFLAGS= -std=c11 -Wall -Wextra $(IDIR)

LDIR = -L/usr/local/lib  -L/usr/lib/x86_64-linux-gnu

LIBS = -lusb-1.0 -lpthread -Wl,-rpath -Wl,/usr/local/lib -lhidapi-libusb #

TARGET = G203

all: $(TARGET)

$(TARGET): $(CFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(CFILES) $(LDIR) $(LIBS)

debug: $(CFILES)
	$(CC) $(CFLAGS) -g -o $(TARGET) $(CFILES) $(LDIR) $(LIBS)

clean:
	$(RM) $(TARGET)
