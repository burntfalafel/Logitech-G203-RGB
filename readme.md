# G203colorshifter
### An onboard RGB/DPI memory management for Logitech's G203 Prodigy.

A C app for changing the color for the G203 Prodigy mouse. Logitech thought it would be a great idea to release RGB/DPI changing software for MacOS and Windows only. Forgot about us Linux folk.

How to use:
```
 user@user-domain >>> G203colorshifter -h
 Logitech G203 Prodigy mouse LED control

 Arguments
     -h, --help                                      Show this help message and exit
     -c, --cycle <rate>,<brightness>                 Cycles the RGB color for the mouse
     -b. --breathe <color>,<rate>,<brightness>       Breathes a RGB color with rate a brightness
     -o, --off                                       Switches off the RGB color for the mouse
     -s, --solid <color>                             Solid color used for the mouse - has to be hex
     -d, --dpi                                       dpi value to be set for the mouse - has to be decimal


    Limits:
                color:RRGGBB
                rate:1000-60000(number of milliseconds)
                brightness:1-100

    Examples:
                G203colorshifter -c 10000,100
                G203colorshifter -b FF000,10000,100
```

## Build from source

Use the Makefile supplied as:
```
make G203colorshifter
make install
```
To remove
```
make clean
make uninstall
```
## What about a full replication of Logitech's Onboard Memory Manager

A replica has been made but a full duplication in functionality for Linux is not possible because of hash values (discussed [here](https://gitlab.com/wady101/rgb-logitech-g203/-/tree/master/etc/readme.md) by me). Your best bet would be to use a Windows VM.
```
make G203MMO
```

Though, G203colorshifter should do most of what you want do anyways.

## What about a full replication of Logitech's G-Hub

Deprecated software written [here](https://gitlab.com/wady101/rgb-logitech-g203/-/tree/logitech-g-hub)
