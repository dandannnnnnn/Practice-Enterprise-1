/* 
 * File:   bluetooth.h
 * Author: Danie
 *
 * Created on May 14, 2026, 2:50 PM
 */

#ifndef BLUETOOTH_H
#define	BLUETOOTH_H

#ifdef	__cplusplus
extern "C" {
#endif
//TX = PD2
//RX = PD3
//1N8 : no parity, 8 datab, 1 stop bit


/*
 * UBBR1H = (unsigned char) (baud >> 8);
 * UBBR1L = (unsigned char) baud;
 * 
 * =======================================
 * =======================================
 * UCSR1B register
 * 
 * RXCIE (Receive Complete Interrupt Enable) = 0 (off) ==> als bit = 1 danwordt atmega onderbroken wanneer data binnenkomt
 * TXCIE = 1 (aan)
 * UDRIEn niet ingesteld (automatisch ingesteld door microcontroller)
 * RXENn = 0 (off) Receiver enable niet nodig
 * TXENn = 1 (aan) Transmitter enable wel nodig
 * UCSZ = instellen voor frame format 8N1 (8 databits, no parity, 1 stopbit)
 * 
 * 
 * RXB8n = 0 (off) ==> niet nodig
 * TXB8n = 0 ==> niet nodig
 * 
 * =======================================
 * =======================================
 * UCSR1A register
 * 
 * RXCn = 0 (off) ==> want RXCIE niet gebruikt
 * TXCn = 1 (aan) ==> TXCIE wel gebruikt
 * UDREn = gebeurt automatisch
 * U2X1 = 1 (async mode)
 * MPCMn = uitzetten ==> niet nodig want receiver is uitgeschakeld
 * 
 * 
 * =======================================
 * =======================================
 * UCSR1C register
 * 
 * UMSEL11 = 0 (async usart)
 * UMSEL10 =  0 (async usart)
 * UPM11 = 0 (no parity)
 * UPM10 = 0 (no parity)
 * USBS1 = 0 (selecteert 1 stop bit)
 * UCSZ12 = 0 (off) // UCSZ11 = 1 (on) // UCSZ10 = 1 (on) ==> 8 databits
 * UCPOL1 = 0 (async mode)
 * =======================================
 * =======================================
 * UCSR1D register
 * 
 * CTSEN = 0 (CTS pin is al in gebruik door led)
 * RTSEN = 0 (RTS pin in gebruik door keyboard switch)
 * 
 * =======================================
 * =======================================
 * 
 */
    
 /* Ingestelde bits voor interrupt (datasheet atmega32U4)
  *
  * Global Interrupt Enable bit = 1 (on) om interrupt in te schakelen
  *  Timer1 (16-bit) gebruikt voor nauwkeurige ms timing p.131 datasheet atmega32u4
  * 
  * Ticks = (16MHz/prescaler) / 1000ms
  * 1ms behalen: prescaler is dan 64 => 250 ticks voor 1ms
  * 
  * (16MHz/64 presc) / 1000ms = 250 ticks voor 1ms
  * 
  * met 250 ticks, kunnen we max. 65 sec delay opzetten (grote range)
  * 
  * TCCR1A = 0x00 ==> uitzetten want PWM is onnodig bij UART
  * 
  * 
  * 
  */   
    
#include <avr/io.h>
#include <stdint.h>
    
    void uart1_init(void) {
        UCSR1A |= (1 << TXC1);
        UCSR1A &= ~((1 << RXC1) | (1 << MPCM1) | (1 << U2X1));
        
        UCSR1B |= (1 << TXCIE1) | (1 << TXEN1);
        UCSR1B &= ~((1 << RXCIE1) | (1 << RXEN1) | (1 << RXB81) | (1 << TXB81));
        
        UCSR1C |= (1 << UCSZ11) | (1 << UCSZ10);
        UCSR1C &= ~((1 << UMSEL11) | (1 << UMSEL10) | (1 << UPM11) | (1 << UPM10) | (1 << USBS1) | (1 << UCSZ12) | (1 << UCPOL1));
    
        UCSR1D &= ~((1 << CTSEN) | (1 << RTSEN));
    }

    void timer1_interrupt(uint16_t ms) {
        TCCR1A = 0x00; //niet nodig want PWM wordt niet gebruikt bij UART
        
        TCCR1B |= (1 << WGM12) |(1 << CS11) | (1 << CS10); //CTC aan en instellen op 64 prescaler
        TCCR1B &= ~(1 << WGM13);
        
        OCR1A = 249; //timer begint bij 0
        
        for (uint16_t i = 0; i < ms; i++) {
            while (!(TIFR1 & (1 << OCF1A))); //wachten tot OCF1A (compare match flag) hoog is
            
            TIFR1 |= (1 << OCF1A);
        }
        TCCR1B = 0;
    }

    
#ifdef	__cplusplus
}
#endif

#endif	/* BLUETOOTH_H */

