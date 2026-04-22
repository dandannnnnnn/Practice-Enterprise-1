/* 
 * File:   USB_header.h
 * Author: Danie
 *
 * Created on March 24, 2026, 12:03 PM
 */

#ifndef USB_HEADER_H
#define	USB_HEADER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>    
    
//Registers
#define USBCON (*(volatile uint8_t *) (0xD8))  
#define USBINT (*(volatile uint8_t *) (0xDA))
#define USBSTA (*(volatile uint8_t *) (0xD9))
    
#define UDCON (*(volatile uint8_t *) (0xE0))
#define UDINT (*(volatile uint8_t *) (0xE1))
#define UDIEN (*(volatile uint8_t *) (0xE2))
#define UDADDR (*(volatile uint8_t *) (0xE3))
#define UDFNUML (*(volatile uint8_t *) (0xE4)) //Geen bit pos.
#define UDFNUMH (*(volatile uint8_t *) (0xE5)) //Geen bit pos.
#define UDMFN (*(volatile uint8_t *) (0xE6)) //Geen bit pos.
#define UENUM (*(volatile uint8_t *) (0xE9))
#define UERST (*(volatile uint8_t *) (0xEA))
#define UECONX (*(volatile uint8_t *) (0xEB))
#define UECFG0X (*(volatile uint8_t *) (0xEC))
#define UECFG1X (*(volatile uint8_t *) (0xED))
#define UESTA0X (*(volatile uint8_t *) (0xEE)) //Geen bit pos.
#define UESTA1X (*(volatile uint8_t *) (0xEF)) //Geen bit pos.
#define UEINTX (*(volatile uint8_t *) (0xE8)) //Geen bit pos.
#define UESTA1X (*(volatile uint8_t *) (0xEF))
#define UEIENX (*(volatile uint8_t *) (0xF0))
#define UEDATX (*(volatile uint8_t *) (0xF1)) //Geen bit pos.
#define UEBCLX (*(volatile uint8_t *) (0xF2)) //Geen bit pos.
#define UEBCHX (*(volatile uint8_t *) (0xF3)) //Geen bit pos.
#define UEINT (*(volatile uint8_t *) (0xF4)) //Geen bit pos.
    
#define PLLCSR (*(volatile uint8_t *) (0x29)) //Nodig want USB heeft 48MHz nodig
#define PLLFRQ (*(volatile uint8_t *) (0x32)) 

//Bit pos. USBCON Reg.
#define USBE 7
#define FRZCLK 5
#define OTGPADE 4
#define VBUSTE 0
    
//Bit pos. USBINT Reg.
#define VBUSTI 0
    
//Bit pos. USBSTA
#define ID 1
#define VBUS 0
    
//Bit Positie UDCON Register
#define RSTCPU 3
#define LSM 2
#define RMWKUP 1
#define DETACH 0

//Bit Positie UDINT Reg
#define UPRSMI 6
#define EORSMI 5
#define WAKEUPI 4
#define EORSTI 3
#define SOFI 2
#define SUSPI 0
    
//Bit Positie UDIEN Reg
#define UPRSME 6
#define EORSME 5
#define WAKEUPE 4
#define EORSTE 3
#define SOFE 2
#define SUSPE 0
    
//Bit Positie UDADDR Reg
#define ADDEN 7
#define UADD6 6
#define UADD5 5
#define UADD4 4    
#define UADD3 3     
#define UADD2 2    
#define UADD1 1
#define UADD0 0

//Bit Pos. UENUM Reg
#define EPNUM2 2
#define EPNUM1 1
#define EPNUM0 0
    
//Bit Pos. UERST Reg
#define EPRST6 6
#define EPRST5 5
#define EPRST4 4
#define EPRST3 3    
#define EPRST2 2
#define EPRST1 1
#define EPRST0 0
    
//Bit Pos. UECONX Reg
#define STALLRQ 5
#define STALLRQC 4
#define RSTDT 3
#define EPEN 0

//Bit Pos. UECFG0X
#define EPTYPE7 7
#define EPTYPE6 6
#define EPDIR 0
    
//Bit Positie UECFG1X
#define EPSIZE6 6
#define EPSIZE5 5
#define EPSIZE4 4
#define EPBK3 3
#define EPBK2 2
#define ALLOC 1
 
//Bit Pos. UEIENX
#define FLERRE 7
#define NAKINE 6
#define NAKOUTE 4
#define RXSTPE 3
#define RXOUTE 2
#define STALLEDE 1
#define TXINE 0
    
//Bit Pos. PLLCSR Reg.
#define PINDIV 4 //bit op 1 voordat PLL wordt enabled (1:2)
#define PLLE 1
#define PLOCK 0    

//Bit Pos. PLLFRQ Reg.
#define PINMUX 7
#define PLLUSB 6
#define PLLTM1 5
#define PLLTM0 4
#define PDIV3 3
#define PDIV2 2
#define PDIV1 1
#define PDIV0 0 

#ifdef	__cplusplus
}
#endif

#endif	/* USB_HEADER_H */

