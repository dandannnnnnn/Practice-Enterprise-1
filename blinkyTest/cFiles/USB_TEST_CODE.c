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
#include <string.h>
#include <util/delay.h>
#include "../headerFiles/init_headerfile.h"
#include "../headerFiles/keys.h"


#define ROWS 7 //input
#define COLS 9 //output

// Function Prototypes
void updateKeyType(uint8_t key, uint8_t pressedKey); 
void setupCols(uint8_t col, uint8_t status);
void matrixScan(void);

void setupHardware(void);


extern volatile uint8_t usbReady;

keyType globalReport;
keyType prevReport; 

uint8_t keyMap[ROWS][COLS] = {
    {HID_KEY_ESCAPE,      HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE},
    {HID_KEY_TAB,         HID_KEY_A, HID_KEY_Z, HID_KEY_E, HID_KEY_R, HID_KEY_T, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE},
    {HID_KEY_LeftControl, HID_KEY_Q, HID_KEY_S, HID_KEY_D, HID_KEY_F, HID_KEY_G, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE},
    {HID_KEY_LeftShift,   HID_KEY_W, HID_KEY_X, HID_KEY_C, HID_KEY_V, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE},
    {HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE},
    {HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE},
    {HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE},
};

int main() {
    setupHardware();
    for(int i = 0; i < 6; i++) 
    {
    globalReport.keys[i] = 0;
    prevReport.keys[i] = 0;
    }
    cols(); //keys.h
    rows(); //keys.h
      for (uint8_t i=0; i<COLS; i++){
        setupCols(i,0); //beginnen met kolom = LOW
    }
    
    while(42)
    {        
        matrixScan();
        PORTD &= ~((1 << PORTD5) | (1 << PORTD0));
    }
    
        
    
    return (EXIT_SUCCESS);
}


void updateKeyType(uint8_t key, uint8_t pressedKey) {
    if (key == 0) { //geen toets dan stopt updateKeyType()
        return;
    }    
     PORTD |= (1 << PORTD5) | (1 << PORTD0);
     
    if (key >= 0xE0 && key <= 0xE7) { //modKey ja? nee?
        if (pressedKey) {
            globalReport.modKeys |= (1 << (key - 0xE0)); //modKey gedetecteerd ==> modKey byte op 1
        } else {
            globalReport.modKeys &= ~(1 << (key - 0xE0)); //geen detectie ==> modKey byte op 0
        }
    } else { //normale keys
        if (pressedKey) 
        {
            for (uint8_t i=0 ; i < 6; i++) {
                if (globalReport.keys[i] == key) { //key al in lijst, er gebeurt niks
                    return;
                }
                if (globalReport.keys[i] == 0) {
                    globalReport.keys[i] = key; //eerste lege plaats zoeken en daar key code bewaren
                    return;
                }
            }
        } else{ //key losgelaten
            for (uint8_t i=0; i<6; i++) {
                if (globalReport.keys[i] == key) { //losgelaten key zoeken en verwijderen uit lijst
                    globalReport.keys[i] = 0;
                }
            }
        }
    }
}

void setupCols(uint8_t col, uint8_t status) {
    if (status) { //staus = 1? ==> kolom-pin = HIGH
        if (col == 0) {
            C1_PORT |= (1<<C1_BIT);
        } else if (col == 1) {
            C2_PORT |= (1 << C2_BIT);
        }else if (col == 2) {
            C3_PORT |= (1 << C3_BIT);
        }else if (col == 3) {
            C4_PORT |= (1 << C4_BIT);
        }else if (col == 4) {
            C5_PORT |= (1 << C5_BIT);
        }else if (col == 5) {
            C6_PORT |= (1 << C6_BIT);
        }else if (col == 6) {
            C7_PORT |= (1 << C7_BIT);
        }else if (col == 7) {
            C8_PORT |= (1 << C8_BIT);
        }else if (col == 8) {
            C9_PORT |= (1 << C9_BIT);
        }
    } else { //status = 0 ? ==> kolom-pin = LOW
        if (col == 0) {
            C1_PORT &= ~(1<<C1_BIT);
        } else if (col == 1) {
            C2_PORT &= ~(1 << C2_BIT);
        }else if (col == 2) {
            C3_PORT &= ~(1 << C3_BIT);
        }else if (col == 3) {
            C4_PORT &= ~(1 << C4_BIT);
        }else if (col == 4) {
            C5_PORT &= ~(1 << C5_BIT);
        }else if (col == 5) {
            C6_PORT &= ~(1 << C6_BIT);
        }else if (col == 6) {
            C7_PORT &= ~(1 << C7_BIT);
        }else if (col == 7) {
            C8_PORT &= ~(1 << C8_BIT);
        }else if (col == 8) {
            C9_PORT &= ~(1 << C9_BIT);
        }
    }
}



void matrixScan(void) {
    for (uint8_t c = 0; c < COLS; c++) {
        setupCols(c,1); //kolom = HIGH
        _delay_us(1O);   // add this
        
        updateKeyType(keyMap[0][c], (R1_PIN & (1 <<R1_BIT)));
        updateKeyType(keyMap[1][c], (R2_PIN & (1 <<R2_BIT)));
        updateKeyType(keyMap[2][c], (R3_PIN & (1 <<R3_BIT)));
        updateKeyType(keyMap[3][c], (R4_PIN & (1 <<R4_BIT)));
        updateKeyType(keyMap[4][c], (R5_PIN & (1 <<R5_BIT)));
        updateKeyType(keyMap[5][c], (R6_PIN & (1 <<R6_BIT)));
        updateKeyType(keyMap[6][c], (R7_PIN & (1 <<R7_BIT)));
        
        setupCols(c,0); //kolom = LOW
    }
}

void setupHardware(void) {
    DDRD |= (1 << DDD5) | (1 << DDD0);
    PORTD &= ~((1 << PORTD5) | (1 << PORTD0));
}

