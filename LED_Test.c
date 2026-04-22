/*
 * File:   LED_Test.c
 * Author: Danie
 *
 * Created on April 22, 2026, 1:27 PM
 */

#include <avr/io.h>
#include <stdint.h>
#include "USB_header.h"

#define LED1_PIN    PD5
#define LED3_PIN    PD0 
#define LED_DDR     DDRD
#define LED_PORT    PORTD

// Bij 16MHz zorgt 1.000.000 loops voor ongeveer 0,5 tot 1 seconde vertraging.
void manual_delay(uint32_t loops) {
    for (volatile uint32_t i = 0; i < loops; i++) {
        __asm__("nop"); 
    }
}

void setupPLL(void) {
    PLLCSR = (1 << PINDIV);
    PLLCSR |= (1 << PLLE); 
    
   
    // PDIV2: 16MHz -> 48MHz
    PLLFRQ |= (1 << PDIV2);
    
    while(!(PLLCSR & (1 << PLOCK))); 
}

void initUSB(void) {
    USBCON |= (1 << USBE);
    USBCON &= ~(1 << FRZCLK);
    
    setupPLL();
    
    USBCON |= (1 << OTGPADE); 
    UDCON &= ~(1 << DETACH); // Verbind met USB host
}

void setupHardware(void) {
    LED_DDR |= (1 << LED1_PIN) | (1 << LED2_PIN);
    
    // LEDs uit bij start
    LED_PORT &= ~((1 << LED1_PIN) | (1 << LED2_PIN));
}

int main(void) {
    setupHardware();
    initUSB();

    // Debugging loop: Als dit werkt, draait je code en werkt je kristal/klok.
    while(1) {
        // Toggle LED 1
        LED_PORT |= (1 << LED1_PIN);
        LED_PORT &= ~(1 << LED2_PIN);
        manual_delay(1000000);

        // Toggle LED 2
        LED_PORT &= ~(1 << LED1_PIN);
        LED_PORT |= (1 << LED2_PIN);
        manual_delay(1000000);
        
        // Hier kun je later de USB taken aanroepen:
        // usb_poll_enumeration();
    }

    return 0;
}
