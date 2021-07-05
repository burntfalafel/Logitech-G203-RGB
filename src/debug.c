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

