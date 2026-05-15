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
 *  UDINT
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
 * UEIENx (endpoint interrupt)
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
 * 
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
 *********************************************************************************
 USB SETUP
 * *******************************************************************************
 * 
 * bladzijde 260
 * 
 * USBCON = USB configuratie regiser
 * 
 * UVREGE = USB pad Regulator --> moet aan vr usb communicatie
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
 * 
 * 
 * Power On the USB interface
? Power-On USB pads regulator
? Configure PLL interface
? Enable PLL
? Check PLL lock
? Enable USB interface
? Configure USB interface (USB speed, Endpoints configuration...)
? Wait for USB VBUS information connection
? Attach USB device
 */

void startupUSB(void) {
    
    USBCON = 0;
    UDCON = 0;
   
    USBCON |= (1 << UVREGE) | (1 << USBE)| (1 << FRZCLK);
    
    
    startupPLL();
    
    UDCON |= (1 << LSM);
    
    USBCON &= ~(1 << FRZCLK);
    UDCON &= ~(1 << DETACH);
}


void shutdownUSB(void) {
    
    UDCON |= (1 << DETACH);
    
    USBCON &= ~(1 << USBE);
   //USBCON &= (1 << FRZCLK);
    
    shutdownPLL();
    
   
    USBCON &= ~(1 << UVREGE); 
}

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


const struct deviceDescriptor PROGMEM USBDeviceDescriptor ={
    .bLength = sizeof(struct deviceDescriptor),
    .bDescriptorType = 0x01,
    .bcdUSB = 0x0200,
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




