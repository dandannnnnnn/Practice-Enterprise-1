/*
 * File:   inputDectection_code.c
 * Author: Danie
 *
 * Created on March 18, 2026, 4:47 PM
 */

#include <avr/io.h>
#include "inputDetect_header.h"
#include "UART_header.h"

#define ROWS 4
#define COLS 10

// columns = input ; rows = output
//DDR '0' = input ; DDR '1' = output
//Portx '1' = High (5V) ; Portx '0' = Low (GND)

char keyMap[ROWS][COLS] = { 
    {'1', '2', '3', '4', '5',                           '6', '7', '8', '9', '0'},
    {'a', 'z', 'e', 'r', 't',                             'y', 'u', 'i', 'o', 'p'},
    {'q', 's', 'd', 'f', 'g',                            'h', 'j', 'k', 'l', 'm'},
    {'w', 'x', 'c', 'v', 'b',                           'n', ',', ';', ':', '='}
};

uint8_t keystatePressed[ROWS][COLS] = {0}; //begin status niet ingedrukt (0)
uint8_t pinsCOL[] = {PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7, PD7};
uint8_t pinsROW[] = {PF0, PF1, PF4, PF5, PF6, PF7, PC7};


void pinsHigh(void) { //cols High
   
    PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB5) | (1 << PB6) | (1 << PB7) ;
    PORTD |= (1 << PD7) ;
}

void pinsLow(void) { //rows Low
    PORTC &= ~((1 << PC7));
    PORTF &= ~((1 << PF0) | (1 << PF1) | (1 << PF4) | (1 << PF5) | (1 << PF6) | (1 << PF7));
}

uint8_t scanKeys() { 
    PORTD &= ~((1 << PD7));
    
   
    
}

int main(void) {
}
