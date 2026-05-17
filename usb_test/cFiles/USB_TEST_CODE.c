/* 
 * File:   USB_TEST_CODE.c
 * Author: stephcuv
 *
 * Created on May 16, 2026, 11:21 PM
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "../headerFiles/init_headerfile.h"

// Function Prototypes
void manual_delay(volatile uint32_t loops);
void setupHardware(void);
void sendHello(void);

int main() {
    startupUSB();
    manual_delay(2000);
    setupHardware();

    while(1) {
        PORTD |= (1 << PORTD5) | (1 << PORTD0);
        manual_delay(2000);
        
        sendHello();
        PORTD &= ~((1 << PORTD5) | (1 << PORTD0));
        manual_delay(2000);
    }
    return (EXIT_SUCCESS);
}

void manual_delay(volatile uint32_t loops) {
    while(loops--) {
        __asm__("nop");
    }
}

void setupHardware(void) {
    DDRD |= (1 << DDD5) | (1 << DDD0);
    PORTD &= ~((1 << PORTD5) | (1 << PORTD0));
}

