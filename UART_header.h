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

#include <avr/io.h>
#include <stdint.h>

#define BAUD 9600
#define MYUBRR (((F_CPU / (BAUD * 16UL))) - 1)

// Functie prototypes
void UART_init(unsigned int ubrr);
void UART_send(uint16_t data);
uint16_t UART_receive(void);
    
#ifdef	__cplusplus
}
#endif

#endif	/* UART_HEADER_H */



