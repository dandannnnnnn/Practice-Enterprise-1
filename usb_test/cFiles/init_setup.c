/*
 * File:   setup.c
 * Author: stephcuv
 *
 * Created on May 13, 2026, 10:02 PM
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "init_headerfile.h"
  /*-----------------------------------------------------------------------------------------------
   * timers/counters
   * pp 92
   * TCCR1B = timer 1
   * 
   * CSn = clock selection bit
   * 000 = no clock (uit)
   * 001 = CLK/1        101 = CLK/1024
   * 010 = CLK/8        110 = external clock falling edge
   * 011 = CLK/64       111 = external clock rizing edge 
   * 100 = CLK/256
   * 
   * WGMn = waveform generation mode = stelt modus timer in
   * counter = 1 time counter
   * (CTC) clear timer on compare = reset counter bij compare
   *  0100 - OCRn (Output Compare Register) comparen tov timervalue 
   *  1100 - ICRn (Input Capture Register) tijd van extern event opslaan 
   * 
   * PWM 
   * 
   * TIMSKn = timerinterupt 1 mask
   * OCIE 3:1 = compair interupts
   * C = output compare C reg
   * B = output compare B reg
   * A = output compare A reg sys
   * 
   * OCRnA = value waarnaar teller count
  */


void timer1setup(void)
{
    //reset timer1 + interupt mask
    TCCR1B = 0;
    TIMSK1 = 0; 
    OCR1A = 0xFFFF; 
    
    TCCR1B = (1 << CS10)| (1 << WGM12);
    
    TIMSK1 =(1 << OCIE1A);
}



 
  /*-----------------------------------------------------------------------------------------------
   * pwm
   * pp  100
  */







/*---------------------------------------------------
 PLL setup

 * PLLCSR = PLL control register
 * 
 * PLLUSB = 0 vr 48 MHz (nodig vr usb communicatie)
 * PLLE = PLL enable 
 * PINDIV = 1 vr 16MHz clock
 * PLOCK = locked ingestelde frequentie
 */
void startupPLL(void) {
    PLLCSR = (1 << PINDIV) | (1 << PLLE);
    while (!(PLLCSR & (1 << PLOCK))); //duurt paar ms voor PLOCK aan staat
}

void shutdownPLL(void) {
      
    PLLCSR &= ~((1 << PINDIV) | (1 << PLLE));
    //while ((PLLCSR & (0 << PLOCK)));
}
/*---------------------------------------------------*/

/*--------------------------------------------------------------
 USB SETUP
 * bladzijde 260
 * 
 * USBCON = USB configuratie regiser
 * 
 * UVREGE = USB pad Regulator --> moet aan vr usb communicatie
 * 
 * USBE = Set to enable the USB controller. Clear to disable and reset the USB controller, 
 * to disable the USB transceiver and to disable the USB controller clock inputs.
 * 
 * FRZCLK = zet interne usb clock uit
 * OTGPADE = VBUS PAD aanzette == vbus detectie
 * VBUS flag = geeft value van vbus terug
 * DETACH = deconnecteerd fysiek usb device    
 * 
 * UDCON LSM = pullup op D+ (high speed 0) pullup op D- (lowspeed 1)
 * 
 * 
 * Power On the USB interface
? Power-On USB pads regulator
? Configure PLL interface
? Enable PLL
? Check PLL lock
? Enable USB interface
? Configure USB interface (USB speed, Endpoints configuration...)
? Wait for USB VBUS information connection
? Attach USB device
 */

void startupUSB(void) {
    
    USBCON = 0;
    UDCON = 0;
   
    USBCON |= (1 << UVREGE) | (1 << USBE)| (1 << FRZCLK);
    
    
    startupPLL();
    
    UDCON |= (1 << LSM);
    
    USBCON &= ~(1 << FRZCLK);
    UDCON &= ~(1 << DETACH);
}


void shutdownUSB(void) {
    
    UDCON |= (1 << DETACH);
    
    USBCON &= ~(1 << USBE);
   //USBCON &= (1 << FRZCLK);
    
    shutdownPLL();
    
   
    USBCON &= ~(1 << UVREGE); 
}


/*------------------------------------------------------------*/


/*-------------------------------------------------------------
 * endpoint setup
 * 
 * endpoint = a data buffer on a USB peripheral device that acts as the 
 * final source or destination for data transferred from or to the host computer
 * 
 * IN endpoint = atmega --> pc
 * OUT endpoint = pc --> atmega
 * 
 * UENUM 2:0= bepaald welke endpoint er gaat gebruikt worden (111b mag niet)
 * 
 * UECONX = STALL shi, data toggle reset, endpoint enable 
 * STALLRQ 5: stall request handshake = STALL answer van host aanvragen
 * STALLRQC 4: stall request clear; 1 STALL handshake uit
 * RSTDT 3: cleared automatisch data toggle 
 * EPEN 0: zet endpoint aan adhv instellingen
 * 
 * 
 * UECFG0X
 * EPTYPE 7:6  = type endpoints
 * 00b = control 
 * 01b = isochronous
 * 10b = Bulk
 * 11b = interrupt
 * 
 * EPDIR 0 = 1 vr IN endpoint 0 vr OUT endpoint
 * 
 * UECFG1X
 * EPSIZE 6:4 = size van endpoint
 * 000b = 8 bytes        100b = 128 bytes
 * 001b = 16 bytes       101b = 256 bytes
 * 010b = 32 bytes       110b = 512 bytes
 * 011b = 64 bytes       111b = restricted
 * 
 * EPBK 3:2 = grootte van bank (keuze adhv endpoint size)
 * 00b = 1 bank
 * 01b = 2 banks
 * 
 * ALLOC 1 = allocate endpoint mem
 *  
 */

//nodig vr usb communicatie (zie blz 270)
void setup0Endpoint(void)
{
    UENUM = 0;
    UECFG0X = 0;
    UECFG1X = 0;
    
    UECFG0X = (0b00 << EPTYPE0) | (0 << EPDIR);
    UECFG1X = (0b011 << EPSIZE0) | (0b00 << EPBK0) | (1 << ALLOC);
}
void setupINEndpoint(void) {
    //endpoint 1
    UENUM = 1;
    UECFG0X = 0;
    UECFG1X = 0;
    
    UECFG0X |= (0b11 << EPTYPE0) | (1 << EPDIR);
    
    //endpoint grootte = 8bytes
    UECFG1X = (0b000 << EPSIZE0) | (0b00 << EPBK0) | (1 << ALLOC);
}
 
void setupOUTEndpoint(void) {
    //endpoint 2
    UENUM = 2;
    UECFG0X = 0;
    UECFG1X = 0;
    
    //interupt mode aanzetten 
    UECFG0X = (0b11 << EPTYPE0) | (0 << EPDIR);
    
    //endpoint grootte = 8bytes
    UECFG1X = (0b000 << EPSIZE0) |(0b00 << EPBK0) | (1 << ALLOC);
}

/*----------------------------------------------------------*/


/*--------------------------------------------------------------------------------
 * delays
 * 
 * 
void delayUs(uint16_t us) {
    while (us--) {
        asm volatile ("nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t");
    }
}
 * 
 * 

void delayMs(uint16_t ms) {
    while (ms--) delayUs(1000);
}
 */