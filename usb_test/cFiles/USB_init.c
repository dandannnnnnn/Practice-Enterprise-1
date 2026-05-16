/* 
 * File:   USB_test_code.c
 * Author: stephcuv
 *
 * Created on May 13, 2026, 10:08 PM
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include<avr/interrupt.h>
#include "../headerFiles/init_headerfile.h"


/*
 * atmega32U4 datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7766-8-bit-AVR-ATmega16U4-32U4_Datasheet.pdf
 * adress setup usb 272
 * 
 * usb 2.0 datasheet https://eater.net/downloads/usb_20.pdf
 * --> extra uitleg https://www.beyondlogic.org/usbnutshell/usb5.shtml#DeviceDescriptors
 * 
 * HID specs https://www.usb.org/sites/default/files/hid1_11.pdf 
 *******************************************************************************
 * INTERUPT STATUS FLAG REGISTERS
 *******************************************************************************
 *
 *  UEINTX geeft events in bepaalde endpoint aan
 * 
 * FIFOCON   7: 
 *             OUT --> nieuw msg in bank op hetzelfde moment als RXOUT of RXSTP
 *                     --> clear bit vr huidige bank los laten + switchen volgende bank
 *             IN  --> indien huidige bank vrij is op zelfde moment als RXIN
 *                     --> clear = send nr FIFO en switch bank
 * 
 * NAKINI    6:  NAK handshake send na IN request host 
 *                --> software clear
 * 
 * RWAL      5: 
 *             IN = bank niet vol --> kan pushe nr fifo
 *             OUT = bank niet vol --> kan leze uit fifo
 *             -nooit gezet indien STALLRQ = 1 of error
 *             -niet vr control endpoint
 * 
 * NAKOUTI  4: NAK handshake sen na OUT/ping request host triggered USB intr
 *             --> software clear
 * 
 * RXSTPI   3: hardware set indien nieuwe valid SETUP packet triggered EPINTx intr
 *             -niet bruikbaar indien IN endpoint
 *              --> software clear
 * 
 * RXOUTI   2: hardware set wnr bank heeft nieuwe packet --> triggered EPINTx intr
 *             --> software clear vr handshake interrupt
 * KILLBK   2: zet bit vr killen current bank
 *             -> hardware clear wnr bank gekilled
 * 
 * STALLEDI 1: hardware set wnr STALL handshake is doorgegaan
 *              --> software clear
 * 
 * TXINI    0: hardware set wnr bank is leeg + triggered EPINTx intr
 *             - inactive wnr OUT endpoint
 *             --> software clear
 * 
 * ---------------------------------------------------------------------------------------------------
 * 
 *  c
 * 
 * WAKEUPI 4: USB - controller re-activated (niet upstream resume) triggered intr indien WAKEUPE 
 *            --> software clear + USB clock input aan
 * EORSTI  3: End Of Reset detected op USB controlle --> triggered USB interrupt indien EORSTE
 *          --> software clear
 * SOFI    2: USB Start Of Frame detected (elke 1ms) triggered USB interrupt indien SOFE
 * SUSPI   0: USB bus is in suspended state
 *         --> software clear 
 * 
 * 
 * ---------------------------------------------------------------------------------------------
 *  UECONX = STALL shi, data toggle reset, endpoint enable
 *  
 * STALLRQ   5: stall request handshake = STALL answer van host aanvragen
 * STALLRQC  4: stall request clear; 1 STALL handshake uit
 * RSTDT     3: cleared automatisch data toggle 
 * EPEN      0: zet endpoint aan adhv instellingen
 * 
 ******************************************************************************
 * INTERUPT ENABLE FLAGS
 * ****************************************************************************
 * 
 * UEIENX (endpoint interrupt)
 * 
 * FLERRE   7: overflow/underflow interrupt
 * NAKINE   6: NAKINI interrupt
 * NAKOUTE  5: NAKOUTI interrupt
 * RXSTPE   3: received SETUP interrupt
 * RXOUTE   2:  RXOUTI interrupt
 * STALLEDE 1:  STALLEDI  interrupt
 * TXINE    0:  TXINI interrupt
 * -------------------------------------------------------------------------------
 * 
 * UDIEN
 * 
 * WAKEUPE 4: WAKEUPI interrupt
 * EORSTE 3: EORSTI interrupt
 * SOFE 2: SOFI interrupt
 * SUSPE 0: SUSPI interrupt
 * 
 * -----------------------------------------------------------------------------
 * 
 */

/******************************************************************************
 * USB endpoint interrupt setup
 * ****************************************************************************
*/

void endpoint0InterruptSetup(void)
{
    UENUM = 0;
    UEIENX = 0;
    
    UEIENX = (1 << RXSTPE) | (1 << RXOUTE) | (1 << TXINE); 
}
void endpointINSetup(void)
{
    UENUM = 1;
    UEIENX = 0;
    
    UEIENX = (1 << TXINE) | (1 << NAKINE) | (1 << STALLEDE) | (1 << FLERRE);
}

void endpointOUTSetup(void)
{
    UENUM = 2;
    UEIENX = 0;
    
    UEIENX = (1 << RXOUTE) | (1 << NAKOUTE) | (1 << STALLEDE) | (1 << FLERRE);
}

void enableEndpointInterrupts(void)
{
    endpoint0InterruptSetup();
    endpointINSetup();
    endpointOUTSetup();
    
    UDIEN = 0;
    UDIEN = (1 << EORSTE);
    
    sei(); //general interrupt aanzetten. 
}

/*
 *******************************************************************************
 * READ/WRITE REGS
 *******************************************************************************
 -----------------------------------------------------------------------------
 * 
 * UEDATx 7:0 = byte vr read/write nr endpoint FIFO
 * 
 * -----------------------------------------------------------------------------
 * 
 * UADDR
 * ADDEN  7: set vr activeren UADD (usb address)
 *         --> hardware clear
 * UADD 6:0: software load vr configureren address
 * 
 * -----------------------------------------------------------------------------
 * 
 * UENUM
 * EPNUM 2:0: bepaald welke endpoint CPU accessed. 
 * 
 * -----------------------------------------------------------------------------
 * 
 * UEINT 
 * EPINT 6:0: welke bit = welke endpoint active
 *********************************************************************************
 USB SETUP
 * *******************************************************************************
 * 
 * bladzijde 260
 * 
 * USBCON = USB configuratie regiser
 * 
 * USBE = Set to enable the USB controller. Clear to disable and reset the USB controller, 
 * to disable the USB transceiver and to disable the USB controller clock inputs.
 * 
 * FRZCLK = zet interne usb clock uit
 * OTGPADE = VBUS PAD aanzette == vbus detectie
 * VBUS flag = geeft value van vbus terug
 * DETACH = deconnecteerd fysiek usb device    
 * 
 * UDCON LSM = pullup op D+ (high speed 0) pullup op D- (lowspeed 1)
 * ---------------------------------------------------------------------------
 * UHWCON
 * 
 * UVREGE = USB pad Regulator --> moet aan vr usb communicatie
 * 
 * 
 * Power On the USB interface
 *  Power-On USB pads regulator
 *  Configure PLL interface
 *  Enable PLL
 *  Check PLL lock
 *  Enable USB interface 
 * Configure USB interface (USB speed, Endpoints configuration...)
 *  Wait for USB VBUS information connection
 *  Attach USB device
 */

void startupUSB(void) {
    
    USBCON = 0;
    UDCON = 0;
    UHWCON = 0;
   
    USBCON = (1 << USBE) | (1 << OTGPADE) | (1 << FRZCLK);
    UHWCON = (1 << UVREGE);
    
    startupPLL();
    
    UDCON |= (1 << LSM);
    
    USBCON &= ~(1 << FRZCLK);
    UDCON &= ~(1 << DETACH);
}

/*
 * Power Off the USB interface
 *  Detach USB interface
 * Disable USB interface
 *  Disable PLL
 *  Disable USB pad regulator
 */
void shutdownUSB(void) {
    
    UDCON |= (1 << DETACH);
    
    USBCON &= ~(1 << USBE) | ~(1 << OTGPADE);
   //USBCON &= (1 << FRZCLK);
    
    shutdownPLL();
    
   
    UHWCON &= ~(1 << UVREGE); 
}

/*
 ****************************************************************************** 
 * ENUMERATION SETPS
 ******************************************************************************
 * 1 Device connects --> address 0
 * 2 Host resets bus --> EORSTI --> re-initialize EP0
 * 3 Host requests Device Descriptor --> reply with class, vendor ID, product ID...
 * 4 Host requests Configuration Descriptor --> reply with config + interface + 
 *   endpoint descriptors chained together
 * 5 Host assigns new address --> UADD + ADDEN
 * 6 Host sends SET_CONFIGURATION --> device is fully enumerated and ready
 * 
 * 
 * 
 *  1: zie blz 264 usb datasheet
 * 
 * __attribute__((packed))--> verwijderd de padding bytes indien toegevoegd tussen
 * velden van de struct.
 * 
 */


/*******************************************************************************
 * REPORT DESCRIPTOR ARRAY
 *******************************************************************************
 * 
 * standaard report descriptor van https://forums.obdev.at/viewtopic4d0c.html?t=10780
 */

const uint8_t PROGMEM keyboardReportDescriptor[] = {
    0x05, 0x01,  // Usage Page (Generic Desktop)
    0x09, 0x06,  // Usage (Keyboard)
    0xA1, 0x01,  // Collection (Application)
    
    // Modifier keys (Ctrl, Shift, Alt...)
    0x05, 0x07,  // Usage Page (Key Codes)
    0x19, 0xE0,  // Usage Minimum (224)
    0x29, 0xE7,  // Usage Maximum (231)
    0x15, 0x00,  // Logical Minimum (0)
    0x25, 0x01,  // Logical Maximum (1)
    0x75, 0x01,  // Report Size (1 bit)
    0x95, 0x08,  // Report Count (8)
    0x81, 0x02,  // Input (Data, Variable, Absolute)
    
    // Reserved byte
    0x95, 0x01,  // Report Count (1)
    0x75, 0x08,  // Report Size (8)
    0x81, 0x01,  // Input (Constant)
    
    // Keycodes (6 keys)
    0x95, 0x06,  // Report Count (6)
    0x75, 0x08,  // Report Size (8)
    0x15, 0x00,  // Logical Minimum (0)
    0x25, 0x65,  // Logical Maximum (101)
    0x05, 0x07,  // Usage Page (Key Codes)
    0x19, 0x00,  // Usage Minimum (0)
    0x29, 0x65,  // Usage Maximum (101)
    0x81, 0x00,  // Input (Data, Array)
    
    0xC0         // End Collection
};


/******************************************************************************
 * STRUCTS
 ******************************************************************************  
 */
struct __attribute__((packed)) deviceDescriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor; //wordt verwacht door pc mr niet nodig vr hobby project --> placeholder
    uint16_t idProduct;//wordt verwacht door pc mr niet nodig vr hobby project --> placeholder
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
};


struct __attribute__((packed)) setupPackage
{
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
};
//zie table 9.10 blz 265 usb datasheet
struct __attribute__((packed)) configurationDescriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
};

struct __attribute__((packed)) HIDDescriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdHID;
    uint8_t bCountryCode;
    uint8_t bNumDescriptors;
    uint8_t bDescriptorType1;
    uint16_t wDescriptorLength;
};

//zie table 9.12 blz 269 usb datasheet
struct __attribute__((packed)) interfaceDescriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iINterface;
};

//zie table 9.13 blz 271 usb datasheet
struct __attribute__((packed)) endpointDescriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
};

struct __attribute__((packed)) configurationPackage
{
    struct configurationDescriptor config;
    struct interfaceDescriptor interface;
    struct HIDDescriptor HID;
    struct endpointDescriptor endpointIN;
    struct endpointDescriptor endpointOUT;
};

/****************************************************************************
 * FILLED IN STRUCTS
 ****************************************************************************
 */
const struct deviceDescriptor PROGMEM USBDeviceDescriptor ={
    .bLength = sizeof(struct deviceDescriptor),
    .bDescriptorType = 0x01,
    .bcdUSB = 0x0200, //version van usb 
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x00,
    .idProduct = 0x00,
    .bcdDevice = 0,
    .iManufacturer = 0,
    .iProduct = 0,
    .iSerialNumber = 0,
    .bNumConfigurations = 1
};
/*
 * descriptor types
 * 0x01: device descriptor
 * 0x02: configuration descriptor
 * 0x03: string descriptor
 * 0x04: interface descriptor
 * 0x05: endpoint descriptor
 * 0x21 HID descriptor
 * 0x22 report descriptor
 */

const struct configurationPackage PROGMEM USBConfigurationPackage = {
    .config = {
        .bLength = sizeof(struct configurationDescriptor),
        .bDescriptorType = 0x02,
        .wTotalLength = sizeof(struct configurationPackage),
        .bNumInterfaces = 1,
        .bConfigurationValue = 1,
        .iConfiguration = 0,
        .bmAttributes = 0x80,
        .bMaxPower = 50,
    },
    //zie https://www.usb.org/sites/default/files/hid1_11.pdf voor instelling
    //interface en subinterface sectie 4.2
    .interface = {
        .bLength = sizeof(struct interfaceDescriptor),
        .bDescriptorType = 0x04,
        .bInterfaceNumber = 0,
        .bAlternateSetting = 0,
        .bNumEndpoints = 2, //aantal endpoints buiten control (standaard)
        .bInterfaceClass = 0x03, //
        .bInterfaceSubClass = 0x01,
        .bInterfaceProtocol = 0x01,
        .iINterface = 0,
    },
    
    .HID = 
    {
      .bLength = sizeof(struct HIDDescriptor),
      .bDescriptorType = 0x21,
      .bcdHID = 0x0111, //current version van HID
      .bNumDescriptors = 1,
      .bCountryCode = 0x00,
      .bDescriptorType1 = 0x22,
      .wDescriptorLength = sizeof(keyboardReportDescriptor),
    },
    /*
     * endpoint address:
     * 7: 1 = IN 0 = out
     * 3:0 = endpoint zelf
     */
    .endpointIN = {
        .bLength = sizeof(struct endpointDescriptor),
        .bDescriptorType = 0x05,
        .bEndpointAddress = 0x81,// 10000001 
        .bmAttributes = 0x02,
        .wMaxPacketSize = 8,
        .bInterval = 1 0,
    },
    .endpointOUT = {
        .bLength = sizeof(struct endpointDescriptor),
        .bDescriptorType = 0x05, 
        .bEndpointAddress = 0x02, //00000010
        .bmAttributes = 0x02,
        .wMaxPacketSize = 8,
        .bInterval = 0,
    }
};


/*
 *******************************************************************************
 * INTERRUPT VECTOR TYPES
 *******************************************************************************
 * zie blz 63 atmega datasheet
 * zie iom32u4.h voor vector namen in C
 * 
 * USB_GEN_vect:USB GENERAL interrupt 
 * USB_COM_vect:USB endpoint interrupt

*******************************************************************************
 * INTERRUPT FUNCTIONS
 ******************************************************************************
 */
// 2: Host reset bus --> EORSTI
ISR(USB_GEN_vect)
{
    if(UDINT & (1 << EORSTI))
    {        
        UDINT &= ~(1 << EORSTI);
        setup0Endpoint();
    }
}

/* 
 * wValue 0x01 = device descriptor 
 * wValue 0x02 = configuration descriptor
 * SET_ADDRESS 0x05
 * GET_DESCRIPTOR 0x06
 * SET_CONFIGURATION 0x09
*/

ISR(USB_COM_vect)
{
    if(UEINT & (1 << EPINT0))
    {
        if(UEINTX & (1 << RXSTPI)) //SETUP package binnen gekomen
        {
            struct setupPackage packet;
            uint8_t *ptr = (uint8_t *)&packet;

            for(uint8_t i = 0; i < sizeof(struct setupPackage); i++) 
            {
                ptr[i] = UEDATX;
            }
            UEINTX &= ~(1 << RXSTPI);
            
            if(packet.bRequest == 0x06)
            {
                //stap 3 device descriptor versturen
              if((packet.wValue >> 8) == 0x01) //<< 8 om higher byte te krijgen
              {
                  while(!(UEINTX & (1 << TXINI)));  // wait until ready

                  const uint8_t *ptr = (const uint8_t *)&USBDeviceDescriptor;
                  for(uint8_t i = 0; i < sizeof(struct deviceDescriptor); i++)
                  {
                      UEDATX = pgm_read_byte(&ptr[i]); //function om te lezen uit flash
                  }
                  UEINTX &= ~(1 << TXINI);          // trigger send
                  while(!(UEINTX & (1 << RXOUTI))); 
                  UEINTX &= ~(1 << RXOUTI);
              }
              
              // stap 4 configuration descriptor versturen 
              else  if((packet.wValue >> 8) == 0x02) //<< 8 om higher byte te krijgen
              {
                  while(!(UEINTX & (1 << TXINI)));
                  
                  const uint8_t *ptr = (const uint8_t *)&USBConfigurationPackage;
                  
                  for(uint8_t i = 0; i < sizeof(struct configurationPackage); i++)
                  {
                      UEDATX = pgm_read_byte(&ptr[i]);
                  }
                  UEINTX &= ~(1 << TXINI);
                  while(!(UEINTX & (1 << RXOUTI)));
                  UEINTX &= ~(1 << RXOUTI);
              } 
              else if((packet.wValue >> 8) == 0x22) //stap 4.5 report descriptor
              {
                while(!(UEINTX & (1 << TXINI)));
                                    
                for(uint8_t i = 0; i < sizeof(keyboardReportDescriptor); i++)
                {
                    UEDATX = pgm_read_byte(&keyboardReportDescriptor[i]);
                }
                UEINTX &= ~(1 << TXINI);
                while(!(UEINTX & (1 << RXOUTI)));
                UEINTX &= ~(1 << RXOUTI); 
              }
            }
            
            //stap 5 SET ADDRESS
            else if(packet.bRequest == 0x05)
            {
                UDADDR = 0;
                UDADDR = (packet.wValue & 0x7F); //nieuw adress zit in lower byte van wValue
                while(!(UEINTX & (1 << TXINI)));  // wait until ready
                UEINTX &= ~(1 << TXINI);
                while(!(UEINTX & (1 << TXINI)));  // wait until ready
                UDADDR |= (1 << ADDEN);
            }
            
            else if(packet.bRequest == 0x0A) // SET_IDLE
            {
                while(!(UEINTX & (1 << TXINI)));
                UEINTX &= ~(1 << TXINI);
                while(!(UEINTX & (1 << TXINI)));
            }
            
            //stap 6 SET CONFIGURATION
            else if(packet.bRequest == 0x09)
            {
                while(!(UEINTX & (1 << TXINI)));  // wait until ready
                UEINTX &= ~(1 << TXINI);
                while(!(UEINTX & (1 << TXINI)));  // wait until ready
                setupINEndpoint();
                setupOUTEndpoint();
            }
        } 
    }
}




/******************************************************************************
 * ONTVANG EN STUUR FUNCTIONS
 ******************************************************************************
 */


void sendData(uint8_t modifier, uint8_t keycode[])
{
    UENUM = 1;
    while(!(UEINTX & (1 << TXINI)));
    UEDATX = modifier;
    UEDATX = 0;
    for(uint8_t i = 0; i<6; i++)
    {
        UEDATX = keycode[i];
    }
    UEINTX &= ~(1 << TXINI);

}