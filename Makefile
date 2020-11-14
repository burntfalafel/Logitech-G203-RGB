CFILES = src/G203.c
IDIR = include/ 
CC = gcc 
CFLAGS= -Wall -Wextra -g -I$(IDIR)

LDIR = /usr/lib/x86_64-linux-gnu

LIBS = -lusb-1.0

TARGET = G203

all: $(TARGET)

$(TARGET): $(CFILES) 
	$(CC) $(CFLAGS) -o $(TARGET) $(CFILES) -L$(LDIR) $(LIBS)

clean: 
	$(RM) $(TARGET)
