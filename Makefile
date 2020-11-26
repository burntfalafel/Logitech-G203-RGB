CFILES = src/G203.c
IDIR = include/
CC = gcc
CFLAGS= -std=c11 -Wall -Wextra -I$(IDIR)

LDIR = /usr/lib/x86_64-linux-gnu

LIBS = -lusb-1.0 -lpthread

TARGET = G203

all: $(TARGET)

$(TARGET): $(CFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(CFILES) -L$(LDIR) $(LIBS)

debug: $(CFILES)
	$(CC) $(CFLAGS) -g -o $(TARGET) $(CFILES) -L$(LDIR) $(LIBS)


clean:
	$(RM) $(TARGET)
