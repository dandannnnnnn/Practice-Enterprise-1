/*
 * File:   inputDectection_code.c
 * Author: Danie
 *
 * Created on March 18, 2026, 4:47 PM
 */

#include <avr/io.h>
#include <util/delay.h>
#include "inputDetect_header.h"
#include "UART_header.h"

// columns = input ; rows = output
//Portx '1' = High (5V) ; Portx '0' = Low (GND)

#define ROWS 7
#define COLS 9

//Linkerduim keys ==> referentie: https://github.com/arduino-libraries/Keyboard/blob/master/src/Keyboard.h en https://www.ascii-code.com/
#define L_ALT 0x82
#define WIN 0x83 //windows button
#define L_ALTGR 0x84
#define SPACE 0x20
#define ENTER 0xE0
#define F1 0xC2
#define F2 0xC3
#define SHIFT 0x87

//Rechterduim keys
#define F3 0xC4
#define F4 0xC5
#define CTRL 0x81
#define R_ALT 0x86
#define R_ALTGR 0x85
#define DELETE 0x7F
#define BACKSPACE 0x08

char keyMap[ROWS][COLS] = { 
    {'1', '2', '3', '4', '5',                           '6', '7', '8', '9', '0'},
    {'a', 'z', 'e', 'r', 't',                             'y', 'u', 'i', 'o', 'p'},
    {'q', 's', 'd', 'f', 'g',                            'h', 'j', 'k', 'l', 'm'},
    {'w', 'x', 'c', 'v', 0,                           'b', 'n', '?', '.', SHIFT},
    {WIN, F1, F2, 0, 0,                       F3, F4, CTRL, 0, 0},
    {SPACE, ENTER, L_ALT, 0, 0       R_ALT, DELETE, BACKSPACE, 0, 0},
    {L_ALTGR, 0, 0, 0, 0                     R_ALTGR, 0, 0, 0, 0}
};

uint8_t keystatePressed[ROWS][COLS] = {0}; //begin status niet ingedrukt (0)
uint8_t pinsCOL[] = {PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7, PD7};
uint8_t pinsROW[] = {PF0, PF1, PF4, PF5, PF6, PF7, PC7};

void setupIO(void) { //kolommen = output
   
  //Rows = Output
    
    DDRF |= (1 << PF7) |(1 << PF6) | (1 << PF5) |(1 << PF4) | (1 << PF1) |(1 << PF0);
    DDRC |= (1 << PC7);
    
  //Cols = Input
    
    DDRD &= ~(1 << PD7);
    DDRB &= ~((1 << PB7) | (1 << PB6) | (1 << PB5) | (1 << PB4) | (1 << PB3) | (1 << PB2) | (1 << PB1) | (1 << PB0));
    
    //Cols = High
    PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB5) | (1 << PB6) | (1 << PB7) ;
    PORTD |= (1 << PD7) ;
    
    //Rows = Low
    PORTC &= ~((1 << PC7));
    PORTF &= ~((1 << PF0) | (1 << PF1) | (1 << PF4) | (1 << PF5) | (1 << PF6) | (1 << PF7));
}

void rowOnLow(uint8_t row, uint8_t active) {
    volatile uint8_t* port = (row < 6) ? &PORTF : &PORTC;
    uint8_t bitMask = (1 << pinsROW[row]);
    
    if (active) {
        *port &= ~bitMask; //Low
    } else {
        *port |= bitMask; //High
    }
}

uint8_t readCol(uint8_t col) {
    if (col == 0) {
        return !(PIND & (1 << PD7));
    } else {
        return !(PINB & (1 << pinsCOL[col]));
    }
}

void matrixScan() {
    for (int r = 0; r < ROWS; r++) {
        rowOnLow(r, 1); //Rij op GND
        __delay_us(10);
        
        for (int c = 0; c < COLS; c++) {
            if (readCol(c)) {
                if (keystatePressed[r][c] == 0) {
                    if (keyMap[r][c] != 0) {
                        UART_send(keyMap[r][c]);
                    }
                    keystatePressed[r][c] = 1;
                }
            } else {
                keystatePressed[r][c] = 0; //niet ingedrukt
            }
        }
        rowOnLow(r, 0);
      }
    }

int main(void) {
    setupIO();
    UART_init();
    while(1) {
        matrixScan();
        __delay_ms(10); //debounce vermijden
        
    }
}