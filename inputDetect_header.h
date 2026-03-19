/* 
 * File:   inputDetect_header.h
 * Author: Danie
 *
 * Created on March 18, 2026, 3:39 PM
 */

#ifndef INPUTDETECT_HEADER_H
#define	INPUTDETECT_HEADER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
    
// columns = input ; rows = output
//DDR '0' = input ; DDR '1' = output
//Portx '1' = High (5V) ; Portx '0' = Low (GND)

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
    
#define R2_DDR DDR_F //poort F
#define R2_PORT portF
#define R2_BIT 4    
#define R2_PIN PINF
    
#define R3_DDR DDR_F //poort F
#define R3_PORT portF
#define R3_BIT 1    
#define R3_PIN PINF
    
#define R4_DDR DDR_F //poort F
#define R4_PORT portF
#define R4_BIT 0   
#define R4_PIN PINF
    
#define R5_DDR DDR_F //poort F
#define R5_PORT portF
#define R5_BIT 6
#define R5_PIN PINF
    
#define R6_DDR DDR_F //poort F
#define R6_PORT portF
#define R6_BIT 7
#define R6_PIN PINF
    
#define R7_DDR DDR_C //poort C
#define R7_PORT portC
#define R7_BIT 7
#define R7_PIN PINF
    
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
    
#ifdef	__cplusplus
}
#endif

#endif	/* INPUTDETECT_HEADER_H */

