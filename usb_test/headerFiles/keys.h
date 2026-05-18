/* 
 * File:   keys.h
 * Author: Danie
 *
 * Created on May 14, 2026, 5:26 PM
 */

#ifndef KEYS_H
#define	KEYS_H

#ifdef	__cplusplus
extern "C" {
#endif

 #include <stdint.h>
#include <avr/io.h>
    
// columns = output; rows = input

#define ROW1 PF5
#define ROW2 PF4
#define ROW3 PF1
#define ROW4 PF0
#define ROW5 PF6
#define ROW6 PF7
#define ROW7 PC7
    
#define COL1 PD7
#define COL2 PB4
#define COL3 PB5
#define COL4 PB6
#define COL5 PB0
#define COL6 PB1
#define COL7 PB2
#define COL8 PB3
#define COL9 PB7

//================================
//================================

//Poorten definiëren Row
#define R1_DDR DDRF //poort F
#define R1_PORT PORTF
#define R1_BIT 5
#define R1_PIN PINF
    
#define R2_DDR DDRF //poort F
#define R2_PORT portF
#define R2_BIT 4    
#define R2_PIN PINF
    
#define R3_DDR DDRF //poort F
#define R3_PORT portF
#define R3_BIT 1    
#define R3_PIN PINF
    
#define R4_DDR DDRF //poort F
#define R4_PORT portF
#define R4_BIT 0   
#define R4_PIN PINF
    
#define R5_DDR DDRF //poort F
#define R5_PORT portF
#define R5_BIT 6
#define R5_PIN PINF
    
#define R6_DDR DDRF //poort F
#define R6_PORT portF
#define R6_BIT 7
#define R6_PIN PINF
    
#define R7_DDR DDRC //poort C
#define R7_PORT portC
#define R7_BIT 7
#define R7_PIN PINF
//==========================================
//==========================================
    
//Poorten definiëren COLS
#define C1_BIT 7
#define C1_PORT PORTD
#define C1_DDR DDRD

#define C2_BIT 4
#define C2_PORT PORTB
#define C2_DDR DDRB

#define C3_BIT 5
#define C3_PORT PORTB
#define C3_DDR DDRB

#define C4_BIT 6
#define C4_PORT PORTB
#define C4_DDR DDRB

#define C5_BIT 0
#define C5_PORT PORTB
#define C5_DDR DDRB

#define C6_BIT 1
#define C6_PORT PORTB
#define C6_DDR DDRB

#define C7_BIT 2
#define C7_PORT PORTB
#define C7_DDR DDRB   
    
#define C8_BIT 3
#define C8_PORT PORTB
#define C8_DDR DDRB  
    
#define C9_BIT 7
#define C9_PORT PORTB
#define C9_DDR DDRB      
    
// ======================================================
// ======================================================
// KEY MAPPING
// ======================================================
// ======================================================
    //Usage ID's mod keys (linkerhelft)
#define HID_KEY_NONE 0x00
#define HID_KEY_LeftShift 0xE1
#define HID_KEY_LeftControl 0xE0
#define HID_KEY_LeftAlt 0xE2
#define HID_KEY_LeftGUI 0xE3

//Rechterhelft    mod keys
#define HID_KEY_RightShift 0xE5
#define HID_KEY_RIghtControl 0xE4
    
//Geen mod keys
#define HID_KEY_ESCAPE 0x29
#define HID_KEY_DELETE 0x2A
#define HID_KEY_TAB 0x2B
#define HID_KEY_SPACEBAR 0x2C 
#define HID_KEY_ENTER 0x58

//Usage ID's Func. keys
#define HID_KEY_F1 0x3A
#define HID_KEY_F2 0x3B

//Usage ID's normale keys
#define HID_KEY_A 0x04
#define HID_KEY_B 0x05
#define HID_KEY_C 0x06
#define HID_KEY_D 0x07
#define HID_KEY_E 0x08
#define HID_KEY_F 0x09
#define HID_KEY_G 0x0A
#define HID_KEY_H 0x0B
#define HID_KEY_I 0x0C
#define HID_KEY_J 0x0D
#define HID_KEY_K 0x0E
#define HID_KEY_L 0x0F
#define HID_KEY_M 0x10
#define HID_KEY_N 0x11
#define HID_KEY_O 0x12
#define HID_KEY_P 0x13
#define HID_KEY_Q 0x14
#define HID_KEY_R 0x15
#define HID_KEY_S 0X16
#define HID_KEY_T 0x17
#define HID_KEY_U 0x18
#define HID_KEY_V 0x19
#define HID_KEY_W 0x1A
#define HID_KEY_X 0x1B
#define HID_KEY_Y 0x1C
#define HID_KEY_Z 0x1D

//numbers
    
#define HID_KEY_1 0x1E
#define HID_KEY_2 0x1F
#define HID_KEY_3 0x20
#define HID_KEY_4 0x21
#define HID_KEY_5 0x22
#define HID_KEY_6 0x23
#define HID_KEY_7 0x24
#define HID_KEY_8 0x25
#define HID_KEY_9 0x26
#define HID_KEY_0 0x27

    
void cols(void) { //output
    C1_DDR |= (1 << C1_BIT);
    C2_DDR |= (1 << C2_BIT);
    C3_DDR |= (1 << C3_BIT);
    C4_DDR |= (1 << C4_BIT);
    C5_DDR |= (1 << C5_BIT);
    C6_DDR |= (1 << C6_BIT);
    C7_DDR |= (1 << C7_BIT);
    C8_DDR |= (1 << C8_BIT);
    C9_DDR |= (1 << C9_BIT);
}

void rows(void) { //input
    R1_DDR &= ~(1 << R1_BIT);
    R2_DDR &= ~(1 << R2_BIT);
    R3_DDR &= ~(1 << R3_BIT);
    R4_DDR &= ~(1 << R4_BIT);
    R5_DDR &= ~(1 << R5_BIT);
    R6_DDR &= ~(1 << R6_BIT);
    R7_DDR &= ~(1 << R7_BIT);
}
    
#ifdef	__cplusplus
}
#endif

#endif	/* KEYS_H */

