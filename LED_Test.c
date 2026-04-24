/*
 * File:   LED_Test.c
 * Author: Danie
 * Modified for: ATmega32U4 using provided iom32u4.h definitions
 */

#include <avr/io.h>
#include "USB_header_1.h"
#include <stdint.h>
#include <util/delay.h>

// Definities op basis van de iom32u4.h namen
#define LED1_BIT    PIND5  // Bit 5 in PORTD/DDRD
#define LED3_BIT    PIND0  // Bit 0 in PORTD/DDRD
#define LED_DDR    DDRD    // Data Direction Register D
#define LED_PORT  PORTD   // Port Register D
//#define DELAY 3000000UL

/*void manual_delay(uint32_t loops) {
    for (volatile uint32_t i = 0; i < loops; i++) {
        __asm__("nop"); 
    }
}*/

void setupPLL(void) {
    // Gebruik de namen uit de header: PLLCSR en de bits PINDIV, PLLE, PLOCK
    PLLCSR = (1 << PINDIV); 
    PLLCSR |= (1 << PLLE);  // Start PLL
    
    // Wacht tot PLL stabiel is (Lock bit)
    while(!(PLLCSR & (1 << PLOCK))); 
    
    // Gebruik PLLFRQ en bit PDIV2 (voor 48MHz bij 8MHz input)
    PLLFRQ = (1 << PDIV2); 
}

void initUSB(void) {
    // Gebruik USBCON en bits USBE, FRZCLK, OTGPADE
    USBCON |= (1 << USBE);    // Enable USB controller
    USBCON &= ~(1 << FRZCLK); // Ontvries USB klok
    
    setupPLL();
    
    USBCON |= (1 << OTGPADE); // Enable VBUS pad
    
    // Gebruik UDCON en bit DETACH
    UDCON &= ~(1 << DETACH);  // Re-attach USB (zorgt dat de PC hem ziet)
}

void setupHardware(void) {
    // Gebruik de DDR bit definities (DDD0, DDD5 etc. zijn ook gedefinieerd als 0 en 5)
    LED_DDR |= (1 << DDB5) | (1 << DDB0); // Let op: in DDRD heten ze DDD5/DDD0 in de header
    // Correctie naar de specifieke header namen voor DDRD:
    DDRD |= (1 << DDD5) | (1 << DDD0);
    
    // LEDs uit bij start
    PORTD &= ~((1 << PORTD5) | (1 << PORTD0));
}

int main(void) {
    //manual_delay(DELAY); 
    _delay_ms(3000);
    setupHardware();
    initUSB();

    while(1) {
        // Toggle LED 1 aan, LED 3 uit
        PORTD |= (1 << PORTD5);
        PORTD &= ~(1 << PORTD0);
        //manual_delay(DELAY);
        _delay_ms(3000);

        // Toggle LED 1 uit, LED 3 aan
        PORTD &= ~(1 << PORTD5);
        PORTD |= (1 << PORTD0);
        //manual_delay(DELAY);
        _delay_ms(3000);
    }

    return 0;
}