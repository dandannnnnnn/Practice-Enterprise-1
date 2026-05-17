/* 
 * File:   USB_init.c
 * Author: stephcuv
 *
 * Created on May 13, 2026, 10:08 PM
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
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
 * UDINT
 * 
 * WAKEUPI 4: USB - controller re-activated (niet upstream resume) triggered intr indien WAKEUPE 
 *            --> software clear + USB clock input aan
 * EORSTI  3: End Of Reset detected op USB controller --> triggered USB interrupt indien EORSTE
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
 * ---------------------------------------------------------------------------------------------
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
    UENUM  = 0;
    UEIENX = 0;
    UEIENX = (1 << RXSTPE); // alleen SETUP during enumeration
}

void endpointINSetup(void)
{
    UENUM  = 1;
    UEIENX = 0;
    UEIENX = (1 << TXINE);
}

void endpointOUTSetup(void)
{
    UENUM  = 2;
    UEIENX = 0;
    UEIENX = (1 << RXOUTE);
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
 *********************************************************************************
 * 
 * bladzijde 260
 * 
 * USBCON = USB configuratie register
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

void startupUSB(void)
{
    USBCON = 0;
    UDCON  = 0;
    UHWCON = 0;

    UHWCON = (1 << UVREGE);
    USBCON = (1 << USBE) | (1 << OTGPADE) | (1 << FRZCLK);

    startupPLL();

    USBCON &= ~(1 << FRZCLK);

    setup0Endpoint();

    UENUM  = 0;
    UEIENX = (1 << RXSTPE);   // alleen SETUP pakkets tijdens enumeration

    UDIEN = 0;
    UDIEN = (1 << EORSTE);

    sei();

    UDCON &= ~(1 << DETACH);
}

/*
 * Power Off the USB interface
 *  Detach USB interface
 *  Disable USB interface
 *  Disable PLL
 *  Disable USB pad regulator
 */
void shutdownUSB(void)
{
    UDCON  |= (1 << DETACH);
    USBCON &= ~((1 << USBE) | (1 << OTGPADE)); // FIX: was ~(A) | ~(B) wat altijd 0xFF geeft
    shutdownPLL();
    UHWCON &= ~(1 << UVREGE);
}

/*
 *******************************************************************************
 * ENUMERATION STEPS
 *******************************************************************************
 * 1 Device connects --> address 0
 * 2 Host resets bus --> EORSTI --> re-initialize EP0
 * 3 Host requests Device Descriptor --> reply with class, vendor ID, product ID...
 * 4 Host requests Configuration Descriptor --> reply with config + interface + 
 *   endpoint descriptors chained together
 * 5 Host assigns new address --> UADD + ADDEN
 * 6 Host sends SET_CONFIGURATION --> device is fully enumerated and ready
 * 
 * __attribute__((packed)) --> verwijderd de padding bytes indien toegevoegd tussen
 * velden van de struct.
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
 ******************************************************************************/

struct __attribute__((packed)) deviceDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
};

struct __attribute__((packed)) setupPackage
{
    uint8_t  bmRequestType;
    uint8_t  bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
};

// zie table 9.10 blz 265 usb datasheet
struct __attribute__((packed)) configurationDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  bMaxPower;
};

// FIX: bCountryCode en bNumDescriptors stonden omgewisseld tov HID spec 1.11 sectie 6.2.1
struct __attribute__((packed)) HIDDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdHID;
    uint8_t  bCountryCode;    // eerst country code
    uint8_t  bNumDescriptors; // dan number of descriptors
    uint8_t  bDescriptorType1;
    uint16_t wDescriptorLength;
};

// zie table 9.12 blz 269 usb datasheet
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

// zie table 9.13 blz 271 usb datasheet
struct __attribute__((packed)) endpointDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bEndpointAddress;
    uint8_t  bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t  bInterval;
};

struct __attribute__((packed)) configurationPackage
{
    struct configurationDescriptor config;
    struct interfaceDescriptor     interface;
    struct HIDDescriptor           HID;
    struct endpointDescriptor      endpointIN;
    struct endpointDescriptor      endpointOUT;
};

/****************************************************************************
 * FILLED IN STRUCTS
 ****************************************************************************/

const struct deviceDescriptor PROGMEM USBDeviceDescriptor = {
    .bLength            = sizeof(struct deviceDescriptor),
    .bDescriptorType    = 0x01,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0,
    .bDeviceSubClass    = 0,
    .bDeviceProtocol    = 0,
    .bMaxPacketSize0    = 64,
    .idVendor           = 0x16C0,  // V-USB test vendor ID
    .idProduct          = 0x05DC,  // V-USB test product ID
    .bcdDevice          = 0,
    .iManufacturer      = 0,
    .iProduct           = 0,
    .iSerialNumber      = 0,
    .bNumConfigurations = 1
};

/*
 * descriptor types
 * 0x01: device descriptor
 * 0x02: configuration descriptor
 * 0x03: string descriptor
 * 0x04: interface descriptor
 * 0x05: endpoint descriptor
 * 0x21: HID descriptor
 * 0x22: report descriptor
 */

const struct configurationPackage PROGMEM USBConfigurationPackage = {
    .config = {
        .bLength             = sizeof(struct configurationDescriptor),
        .bDescriptorType     = 0x02,
        .wTotalLength        = sizeof(struct configurationPackage),
        .bNumInterfaces      = 1,
        .bConfigurationValue = 1,
        .iConfiguration      = 0,
        .bmAttributes        = 0x80,
        .bMaxPower           = 50,
    },
    // zie https://www.usb.org/sites/default/files/hid1_11.pdf voor instelling
    // interface en subinterface sectie 4.2
    .interface = {
        .bLength            = sizeof(struct interfaceDescriptor),
        .bDescriptorType    = 0x04,
        .bInterfaceNumber   = 0,
        .bAlternateSetting  = 0,
        .bNumEndpoints      = 2,
        .bInterfaceClass    = 0x03,
        .bInterfaceSubClass = 0x01,
        .bInterfaceProtocol = 0x01,
        .iINterface         = 0,
    },
    .HID = {
        .bLength           = sizeof(struct HIDDescriptor),
        .bDescriptorType   = 0x21,
        .bcdHID            = 0x0111,
        .bCountryCode      = 0x00,
        .bNumDescriptors   = 1,
        .bDescriptorType1  = 0x22,
        .wDescriptorLength = sizeof(keyboardReportDescriptor),
    },
    /*
     * endpoint address:
     * bit 7: 1 = IN, 0 = OUT
     * bit 3:0 = endpoint nummer
     */
    .endpointIN = {
        .bLength          = sizeof(struct endpointDescriptor),
        .bDescriptorType  = 0x05,
        .bEndpointAddress = 0x81,   // 10000001 = IN endpoint 1
        .bmAttributes     = 0x03,
        .wMaxPacketSize   = 8,
        .bInterval        = 10,
    },
    .endpointOUT = {
        .bLength          = sizeof(struct endpointDescriptor),
        .bDescriptorType  = 0x05,
        .bEndpointAddress = 0x02,   // 00000010 = OUT endpoint 2
        .bmAttributes     = 0x03,
        .wMaxPacketSize   = 8,
        .bInterval        = 0,
    }
};


/*
 *******************************************************************************
 * INTERRUPT VECTOR TYPES
 *******************************************************************************
 * zie blz 63 atmega datasheet
 * zie iom32u4.h voor vector namen in C
 * 
 * USB_GEN_vect: USB GENERAL interrupt 
 * USB_COM_vect: USB endpoint interrupt
 *
 *******************************************************************************
 * INTERRUPT FUNCTIONS
 *******************************************************************************
 */

// stap 2: Host reset bus --> EORSTI
ISR(USB_GEN_vect)
{
    if(UDINT & (1 << EORSTI))
    {
        UDINT &= ~(1 << EORSTI);
        setup0Endpoint();
        //UEIENX opnieuw instellen na reset - setup0Endpoint wist de registers
        UENUM  = 0;
        UEIENX = (1 << RXSTPE);
    }
}

/* 
 * wValue 0x01 = device descriptor 
 * wValue 0x02 = configuration descriptor
 * wValue 0x22 = report descriptor
 * SET_ADDRESS      0x05
 * GET_DESCRIPTOR   0x06
 * SET_IDLE         0x0A
 * SET_CONFIGURATION 0x09
 */

ISR(USB_COM_vect)
{
    if(UEINT & (1 << EPINT0))
    {
        UENUM = 0;

        if(UEINTX & (1 << RXSTPI)) // SETUP pakket binnengekomen
        {
            struct setupPackage packet;
            uint8_t *ptr = (uint8_t *)&packet;

            for(uint8_t i = 0; i < sizeof(struct setupPackage); i++)
            {
                ptr[i] = UEDATX;
            }
            UEINTX &= ~(1 << RXSTPI);

            //bmRequestType controleren voor bRequest - voorkomt verkeerde dispatch
            // stap 3: GET_DESCRIPTOR (device-to-host, standard, device)
            if( packet.bRequest == 0x06)
            {
                if((packet.bmRequestType == 0x80)&&((packet.wValue >> 8) == 0x01)) // device descriptor
                {
                    while(!(UEINTX & (1 << TXINI)));

                    // FIX: clamp sendLen naar wLength - host vraagt eerst 8 bytes
                    //      om bMaxPacketSize0 te lezen, daarna pas alle 18
                    const uint8_t *ptr = (const uint8_t *)&USBDeviceDescriptor;
                    uint8_t sendLen = (packet.wLength < sizeof(struct deviceDescriptor))
                                      ? (uint8_t)packet.wLength : sizeof(struct deviceDescriptor);
                    for(uint8_t i = 0; i < sendLen; i++)
                    {
                        UEDATX = pgm_read_byte(&ptr[i]);
                    }
                    UEINTX &= ~(1 << TXINI);
                    while(!(UEINTX & (1 << RXOUTI)));
                    UEINTX &= ~(1 << RXOUTI);
                }

                // stap 4: configuration descriptor
                else if((packet.bmRequestType == 0x80)&&((packet.wValue >> 8) == 0x02)) // device descriptor
                {
                    while(!(UEINTX & (1 << TXINI)));


                    const uint8_t *ptr = (const uint8_t *)&USBConfigurationPackage;
                    uint8_t sendLen = (packet.wLength < sizeof(struct configurationPackage))? (uint8_t)packet.wLength: sizeof(struct configurationPackage);
                    for(uint8_t i = 0; i < sendLen; i++)
                    {
                        UEDATX = pgm_read_byte(&ptr[i]);
                    }
                    UEINTX &= ~(1 << TXINI);
                    while(!(UEINTX & (1 << RXOUTI)));
                    UEINTX &= ~(1 << RXOUTI);
                }

                // stap 4.5: report descriptor
                if((packet.bmRequestType == 0x81)&&((packet.wValue >> 8) == 0x22)) // device descriptor
                {
                    while(!(UEINTX & (1 << TXINI)));

                    // FIX: clamp sendLen naar wLength
                    uint8_t sendLen = (packet.wLength < sizeof(keyboardReportDescriptor))
                                      ? (uint8_t)packet.wLength
                                      : sizeof(keyboardReportDescriptor);
                    for(uint8_t i = 0; i < sendLen; i++)
                    {
                        UEDATX = pgm_read_byte(&keyboardReportDescriptor[i]);
                    }
                    UEINTX &= ~(1 << TXINI);
                    while(!(UEINTX & (1 << RXOUTI)));
                    UEINTX &= ~(1 << RXOUTI);
                }
                else
                {
                    // onbekend descriptor type --> STALL
                    UECONX = (1 << STALLRQ) | (1 << EPEN);
                }
            }

            // stap 5: SET_ADDRESS (host-to-device, standard, device)
            else if(packet.bmRequestType == 0x00 && packet.bRequest == 0x05)
            {
                UDADDR = (packet.wValue & 0x7F); // nieuw adress zit in lower byte van wValue
                while(!(UEINTX & (1 << TXINI)));
                UEINTX &= ~(1 << TXINI);
                while(!(UEINTX & (1 << TXINI))); // wacht tot STATUS stage klaar is
                UDADDR |= (1 << ADDEN);           // activeer nieuw adres pas daarna
            }

            // SET_IDLE (class request van HID host)
            else if(packet.bmRequestType == 0x21 && packet.bRequest == 0x0A)
            {
                while(!(UEINTX & (1 << TXINI)));
                UEINTX &= ~(1 << TXINI);
                while(!(UEINTX & (1 << TXINI)));
            }

            // stap 6: SET_CONFIGURATION (host-to-device, standard, device)
            else if(packet.bmRequestType == 0x00 && packet.bRequest == 0x09)
            {
                while(!(UEINTX & (1 << TXINI)));
                UEINTX &= ~(1 << TXINI);
                while(!(UEINTX & (1 << TXINI)));
                setupINEndpoint();
                setupOUTEndpoint();
            }

            /* alles wat niet herkend wordt krijgt een STALL
             *  zodat de host weet dat het request niet ondersteund wordt
             *  ipv te wachten op een timeout
             */
            else
            {
                UECONX = (1 << STALLRQ) | (1 << EPEN);
            }
        }
    }

    if(UEINT & (1 << EPINT1))
    {
        // IN endpoint interrupt handler - hier later keyboarddata sturen
    }
}


/******************************************************************************
 * ONTVANG EN STUUR FUNCTIONS
 ******************************************************************************/

void sendData(uint8_t modifier, uint8_t keycode[])
{
    UENUM = 1;
    while(!(UEINTX & (1 << TXINI)));
    UEDATX = modifier;
    UEDATX = 0;
    for(uint8_t i = 0; i < 6; i++)
    {
        UEDATX = keycode[i];
    }
    UEINTX &= ~(1 << TXINI);
}

void sendHello(void)
{
    // HID keycode tabel: https://usb.org/sites/default/files/hut1_3_0.pdf blz 88
    // modifier = 0x00 (geen shift/ctrl/alt)
    // keycode  = 0x00 betekent geen toets ingedrukt
    
    // h = 0x0B
    // e = 0x08
    // l = 0x0F
    // l = 0x0F
    // o = 0x12

    uint8_t keycodes[6] = {0};

    // stuur 'h'
    keycodes[0] = 0x0B;
    sendData(0x00, keycodes);
    keycodes[0] = 0x00;
    sendData(0x00, keycodes);   // toets loslaten

    // stuur 'e'
    keycodes[0] = 0x08;
    sendData(0x00, keycodes);
    keycodes[0] = 0x00;
    sendData(0x00, keycodes);

    // stuur 'l'
    keycodes[0] = 0x0F;
    sendData(0x00, keycodes);
    keycodes[0] = 0x00;
    sendData(0x00, keycodes);

    // stuur 'l'
    keycodes[0] = 0x0F;
    sendData(0x00, keycodes);
    keycodes[0] = 0x00;
    sendData(0x00, keycodes);

    // stuur 'o'
    keycodes[0] = 0x12;
    sendData(0x00, keycodes);
    keycodes[0] = 0x00;
    sendData(0x00, keycodes);
}