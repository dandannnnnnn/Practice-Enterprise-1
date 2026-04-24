/*
 * File:   USBcode.c
 * Author: Danie
 *
 * Created on March 24, 2026, 11:59 AM
 */


#include <avr/io.h>
#include "USB_header.h"
#include <stdint.h>

const uint8_t device_descriptor[] = {
    18,          // Lengte van deze descriptor
    1,           // Type: Device Descriptor
    0x00, 0x02,  // USB Versie 2.00 (Little Endian)
    0, 0, 0,     // Class, Subclass, Protocol (0 = gedefinieerd in Interface)
    64,          // Max Packet Size voor Endpoint 0
    0xEB, 0x03,  // Vendor ID (0x03EB is Atmel)
    0x61, 0x20,  // Product ID (0x2061 is vaak keyboard)
    0x00, 0x01,  // Device Release Number
    0, 0, 0,     // Index fabrikant, product, serienummer (0 = geen)
    1            // Aantal configuraties
};

void setupPLL(void) {
    // Voor 16MHz kristal: PINDIV '1' zijn (input / 2 = 8MHz voor PLL)
    PLLCSR = (1 << PINDIV); 
    PLLCSR |= (1 << PLLE);               // Enable PLL
    while (!(PLLCSR & (1 << PLOCK)));    // Wacht op lock
}

void initUSB(void) {
    USBCON |= (1 << USBE);   //usb controller enable
    USBCON &= ~(1 << FRZCLK);  // Unfreeze clock
    
    setupPLL();
    
    // Attach device
    UDCON &= ~(1 << DETACH);   // hoog zodat zichtbaar voor pc
}

void setupControlEndpoint(void) {
    UENUM = 0; // Selecteer Endpoint 0
    UECONX |= (1 << EPEN); // Enable
    UECFG0X = 0; // Type: Control
    UECFG1X = (1 << EPSIZE1) | (1 << EPSIZE0) | (1 << ALLOC); // 32 bytes, 1 bank
}

void usb_task(void) {
    UENUM = 0; 
    
    if (UEINTX & (1 << RXSTPE)) {
        uint8_t bmRequestType = UEDATX;
        uint8_t bRequest      = UEDATX;
        uint16_t wValue       = UEDATX;
        wValue |= (UEDATX << 8);
        uint16_t wIndex       = UEDATX;
        wIndex |= (UEDATX << 8);
        uint16_t wLength      = UEDATX;
        wLength |= (UEDATX << 8);

        UEINTX &= ~(1 << RXSTPE); // Clear interrupt vlag

        if (bRequest == 0x06) {
            uint8_t descriptor_type = (wValue >> 8);
            if (descriptor_type == 0x01) { // Device Descriptor
                while (!(UEINTX & (1 << TXINE)));
                for (uint8_t i = 0; i < sizeof(device_descriptor); i++) {
                    UEDATX = device_descriptor[i];
                }
                UEINTX &= ~(1 << TXINE); // stuur bank
            }
        }
        
        if (bRequest == 0x05) {
            // ACK  pakket eerst
            UEINTX &= ~(1 << TXINE);
            while (!(UEINTX & (1 << TXINE)));
            
            // Adres instellen
            UDADDR = (wValue & 0x7F);
            UDADDR |= (1 << ADDEN);
        }
        
        if (bRequest == 0x09) {
             UEINTX &= ~(1 << TXINE); // Bevestig config
        }
    }
}

int main(void) {
    // watchdog uit
    MCUSR &= ~(1 << WDRF);
    
    initUSB();
    setupControlEndpoint();

    while (1) {
        usb_task();
    }
}