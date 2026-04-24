/*
 * File:   communicatieCode.c
 * Author: Danie
 *
 * Created on March 13, 2026, 11:57 AM
 */


#include <avr/io.h>
#define F_CPU 16000000UL //16MHz kristal
#include <util/delay.h>
#include "UART_header.h"

void UART_init(unsigned int baud) { //Verloop communicatie
    
    //Baud rate instellen
    UBRR1H = (unsigned char) (baud >> 8);
    UBRR1L = (unsigned char) baud;
    
    //Register A
    UCSR1A = 0x00;
    
    //Register B
    //RXEN & TXEN zetten hw aan ; UCSZ12 9-bit data
    UCSR1B = (1 << RXEN1) | (1 << TXEN1) | (1 << UCSZ12);
    
    //Register C
    //9N1
    UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);
}

void UART_send(uint16_t data) {
    //wachten op data
    while (!(UCSR1A & (1 << UDRE1)));
    
    if (data & 0x0100) {
        UCSR1B |= (1 << TXB81);
    } else {
        UCSR1B &= ~(1 << TXB81); //~ bitwise NOT
    }
    
    UDR1 = (uint8_t) data;
}

uint16_t UART_receive(void) {
    //wachten op data
    while (!(UCSR1A & (1 << RXC1)));
    
    uint8_t bStatus = UCSR1B; //9e bit lezen van UCSR1B
    uint8_t dataL = UDR1;
    
    if (bStatus & (1 << RXB81)) {
        return (0x0100 | dataL);
    } else {
        return dataL;
    }
}

int main(void) {
    UART_init(103); //UBRR-waarde voor 9600 baud 
    uint16_t Datareceived;
    
    while (1) {
        Datareceived = UART_receive();
        UART_send(Datareceived);
    }
    
    return 0;
}