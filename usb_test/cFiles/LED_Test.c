/*
 * File:   LED_Test.c
 * Author: Danie
 * Modified for: ATmega32U4 using provided iom32u4.h definitions
 */

#include <avr/io.h>
#include "USB_header_1.h"
#include <stdint.h>

void manual_delay(volatile uint32_t loops) {
    while(loops--) {
        __asm__("nop");
    }
}

void setupPLL(void) {
    PLLCSR = (1 << PINDIV); 
    PLLCSR |= (1 << PLLE);
    
    while(!(PLLCSR & (1 << PLOCK))); 
    
    PLLFRQ = (1 << PDIV2); 
}

void initUSB(void) {
    USBCON |= (1 << USBE);
    USBCON &= ~(1 << FRZCLK);
    
    setupPLL();
    
    USBCON |= (1 << OTGPADE);
    UDCON &= ~(1 << DETACH);
}

void setupHardware(void) {
    DDRD |= (1 << DDD5) | (1 << DDD0);
    PORTD &= ~((1 << PORTD5) | (1 << PORTD0));
}

int main(void) {
    manual_delay(2000000);
    setupHardware();
    initUSB();

    while(1) {
        PORTD |= (1 << PORTD5) | (1 << PORTD0);
        manual_delay(2000000);
        
        PORTD &= ~((1 << PORTD5) | (1 << PORTD0));
        manual_delay(2000000);
    }

    return 0;
}