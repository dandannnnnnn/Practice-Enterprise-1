/*
 * File:   USBcode.c
 * Author: Danie
 *
 * Created on March 24, 2026, 11:59 AM
 */


#include <avr/io.h>
#include "USB_header.h"

void setupPLL(void) { //datash. p.40
    PLLCSR |= (1 << PINDIV) | (1 < PLLE) | (1 << PLOCK); //init PLL input = 8MHz en output voor BUS is 48MHz
    
    PLLFRQ |= (1 << PDIV2); //bit '1'
    PLLFRQ &= ~((1 << PINMUX) | (1 << PLLUSB) | (1 << PLLTM1) | (1 << PLLTM0) | (1 << PDIV3) | (1 << PDIV1) | (1 << PDIV0)); //bit '0'
}

void setupUSB(void) { //datasheet p.281
    UDCON &= ~((1 << RSTCPU) | (1 << LSM) | );
}

void main(void) {
    return;
}
