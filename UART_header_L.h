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

void UART_init(uint32_t baud);
void UART_send(char data);
    
#ifdef	__cplusplus
}
#endif

#endif	/* UART_HEADER_H */



