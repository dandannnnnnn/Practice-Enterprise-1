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
    
#include <avr/io.h>
#include <stdint.h>
    
    void initUSB(void);
    void setupPLL(void);
    
#ifdef	__cplusplus
}
#endif

#endif	/* USB_HEADER_H */
