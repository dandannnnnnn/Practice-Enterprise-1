/* 
 * File:   UART_header.h
 * Author: Mikhaela Balaga
 *
 * Created on March 13, 2026, 10:41 AM
 */

#ifndef UART_HEADER_H
#define	UART_HEADER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
   
// Registers A, B, C, D
#define UCSR1A (*(volatile uint8_t *) (0xC8))
#define UCSR1B (*(volatile uint8_t *) (0xC9))
#define UCSR1C (*(volatile uint8_t *) (0xCA))
#define UCSR1D (*(volatile uint8_t *) (0xCB)) 
    
//Baud Rate Register
#define UBRR1H (*(volatile uint8_t *) (0xCD)) 
#define UBRR1L (*(volatile uint8_t *) (0xCC)) 
    
#define UDR1 (*(volatile uint8_t *) (0xCE)) // Input/Output Data Register
    
//Bits in Register A
#define RXC1 7      //RXC1 is bit 7 receive complete
#define TXC1 6
#define UDRE1 5
#define FE1 4       // altijd bit op 0 zetten bij het schrijven naar UCSRnA
#define DOR1 3      // altijd bit op 0 zetten bij het schrijven naar UCSRnA
#define UPE1 2      // altijd bit op 0  zetten bij het schrijven naar UCSRnA (uitzetten)
#define U2X1 1       // bit op 0 zetten voor standaard mode async.
#define MPCM1 0     // bit op 1 zetten ==> 9N1 gebruiken, anders gewoon 8N1

 //Bits in Register B
#define RXCIE1 7
#define TXCIE1 6
#define UDRIE1 5
#define RXEN1 4
#define TXEN1 3
#define UCSZ12 2
#define RXB81 1     //9e bit (9N1) Rx
#define TXB81 0     //9e bit Tx
    
//Bits in Register C
#define UMSEL11 7   //Async UART ==> UMSEL op 00 zetten
#define UMSEL10 6       //No parity (9N1) ==> UPM op 00 zetten in code
#define UPM11 5
#define UPM10 4
#define USBS1 3
#define UCSZ11 2
#define UCSZ10 1
#define UCPOL1 0    //moet op 0 ==> async. modus
    
//Bits in Register D
#define CTSEN 1
#define RTSEN 0
    
    
#ifdef	__cplusplus
}
#endif

#endif	/* UART_HEADER_H */



