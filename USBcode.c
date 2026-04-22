/*
 * File:   USBcode.c
 * Author: Danie
 *
 * Created on March 24, 2026, 11:59 AM
 */


#include <avr/io.h>
#include "USB_header.h"
#include <stdint.h>

const uint8_t device_desc[] = {
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
    // Voor 16MHz kristal: PINDIV moet 1 zijn (input / 2 = 8MHz voor PLL)
    PLLCSR = (1 << PINDIV); 
    PLLCSR |= (1 << PLLE);               // Enable PLL
    while (!(PLLCSR & (1 << PLOCK)));    // Wacht op lock
}

void initUSB(void) {
    // USBCON is niet in je header, maar zit in avr/io.h
    USBCON |= (1 << USBE);     // Enable USB controller
    USBCON &= ~(1 << FRZCLK);  // Unfreeze clock
    
    setupPLL();
    
    // Attach device
    UDCON &= ~(1 << DETACH);   // Trek DP/DM lijnen hoog zodat PC de verbinding ziet
}

void setupControlEndpoint(void) {
    UENUM = 0; // Selecteer Endpoint 0
    UECONX |= (1 << EPEN); // Enable
    UECFG0X = 0; // Type: Control
    UECFG1X = (1 << EPSIZE1) | (1 << EPSIZE0) | (1 << ALLOC); // 32 bytes, 1 bank
}

void usb_task(void) {
    UENUM = 0; // Altijd naar EP0 kijken voor configuratie
    
    if (UEINTX & (1 << RXSTPE)) { // Hebben we een Setup Packet ontvangen?
        uint8_t bmRequestType = UEDATX;
        uint8_t bRequest      = UEDATX;
        uint16_t wValue       = UEDATX;
        wValue |= (UEDATX << 8);
        uint16_t wIndex       = UEDATX;
        wIndex |= (UEDATX << 8);
        uint16_t wLength      = UEDATX;
        wLength |= (UEDATX << 8);

        UEINTX &= ~(1 << RXSTPE); // Clear interrupt vlag

        // 1. GET_DESCRIPTOR request
        if (bRequest == 0x06) {
            uint8_t descriptor_type = (wValue >> 8);
            if (descriptor_type == 0x01) { // Device Descriptor
                for (uint8_t i = 0; i < sizeof(device_descriptor); i++) {
                    while (!(UEINTX & (1 << TXINE))); // Wacht tot buffer klaar is
                    UEDATX = device_descriptor[i];
                }
                UEINTX &= ~(1 << TXINE); // Verstuur bank
            }
        }
        
        // 2. SET_ADDRESS request
        if (bRequest == 0x05) {
            // ACK het pakket eerst
            UEINTX &= ~(1 << TXINE);
            while (!(UEINTX & (1 << TXINE)));
            
            // Adres instellen
            UDADDR = (wValue & 0x7F);
            UDADDR |= (1 << ADDEN);
        }
        
        // 3. SET_CONFIGURATION
        if (bRequest == 0x09) {
             UEINTX &= ~(1 << TXINE); // Bevestig configuratie
        }
    }
}

int main(void) {
    // Uitschakelen Watchdog (veiligheid)
    MCUSR &= ~(1 << WDRF);
    
    initUSB();
    setupControlEndpoint();
    
    // Globale interrupts aan indien bij gebruik
    // sei(); 

    while (1) {
        usb_task();
    }
}