/* 
 * File:   USB_test_code.c
 * Author: stephcuv
 *
 * Created on May 13, 2026, 10:08 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "init_headerfile.h"



//adress setup usb 272

/*
 *******************************************************************************
 * INTERUPT REGISTERS
 *******************************************************************************
 *
 *  UEINTX geeft events in bepaalde endpoint aan
 * 
 * FIFOCON   7: 
 *             OUT --> nieuw msg in bank op hetzelfde moment als RXOUT of RXSTP
 *                     --> clear bit vr huidige bank los laten + switchen volgende bank
 *             IN  --> indien huidige bank vrij is op zelfde moment als RXIN
 *                     --> clear = send nr FIFO en switch bank
 * 
 * NAKINI    6:  NAK handshake send na IN request host 
 *                --> software clear
 * 
 * RWAL      5: 
 *             IN = bank niet vol --> kan pushe nr fifo
 *             OUT = bank niet vol --> kan leze uit fifo
 *             -nooit gezet indien STALLRQ = 1 of error
 *             -niet vr control endpoint
 * 
 * NAKOUTI  4: NAK handshake sen na OUT/ping request host triggered USB intr
 *             --> software clear
 * 
 * RXSTPI   3: hardware set indien nieuwe valid SETUP packet triggered EPINTx intr
 *             -niet bruikbaar indien IN endpoint
 *              --> software clear
 * 
 * RXOUTI   2: hardware set wnr bank heeft nieuwe packet --> triggered EPINTx intr
 *             --> software clear vr handshake interrupt
 * KILLBK   2: zet bit vr killen current bank
 *             -> hardware clear wnr bank gekilled
 * 
 * STALLEDI 1: hardware set wnr STALL handshake is doorgegaan
 *              --> software clear
 * 
 * TXINI    0: hardware set wnr bank is leeg + triggered EPINTx intr
 *             - inactive wnr OUT endpoint
 *             --> software clear
 * 
 * ---------------------------------------------------------------------------------------------------
 * 
 *  UDINT
 * 
 * WAKEUPI 4: USB - controller re-activated (niet upstream resume) triggered intr indien WAKEUPE 
 *            --> software clear + USB clock input aan
 * EORSTI  3: End Of Reset detected op USB controlle --> triggered USB interrupt indien EORSTE
 *          --> software clear
 * SOFI    2: USB Start Of Frame detected (elke 1ms) triggered USB interrupt indien SOFE
 * SUSPI   0: USB bus is in suspended state
 *         --> software clear 
 * 
 * 
 * ---------------------------------------------------------------------------------------------
 *  UECONX = STALL shi, data toggle reset, endpoint enable
 *  
 * STALLRQ   5: stall request handshake = STALL answer van host aanvragen
 * STALLRQC  4: stall request clear; 1 STALL handshake uit
 * RSTDT     3: cleared automatisch data toggle 
 * EPEN      0: zet endpoint aan adhv instellingen
 * 
 ******************************************************************************
 * INTERUPT FLAGS
 * ****************************************************************************
 * 
 * UEIENx 
 * 
 * FLERRE   7: overflow/underflow interrupt
 * NAKINE   6: NAKINI interrupt
 * NAKOUTE  5: NAKOUTI interrupt
 * RXSTPE   3: received SETUP interrupt
 * RXOUTE   2:  RXOUTI interrupt
 * STALLEDE 1:  STALLEDI  interrupt
 * TXINE    0:  TXINI interrupt
 * -------------------------------------------------------------------------------
 * 
 * UDIEN
 * 
 * WAKEUPE 4: WAKEUPI interrupt
 * EORSTE 3: EORSTI interrupt
 * SOFE 2: SOFI interrupt
 * SUSPE 0: SUSPI interrupt
 * 
 * -----------------------------------------------------------------------------
 * 
 * 
 *******************************************************************************
 * READ/WRITE REGS
 *******************************************************************************
 -----------------------------------------------------------------------------
 * 
 * UEDATx 7:0 = byte vr read/write nr endpoint FIFO
 * 
 * -----------------------------------------------------------------------------
 * 
 * UADDR
 * ADDEN  7: set vr activeren UADD (usb address)
 *         --> hardware clear
 * UADD 6:0: software load vr configureren address

 * 
 * 
 ****************************************************************************** 
 * ENUMERATION SETPS
 ******************************************************************************
 * 1 Device connects ? address 0
 * 2 Host resets bus ? EORSTI ? re-initialize EP0
 * 3 Host requests Device Descriptor ? reply with class, vendor ID, product ID...
 * 4 Host requests Configuration Descriptor ? reply with config + interface + 
 *   endpoint descriptors chained together
 * 5 Host assigns new address ? UADD + ADDEN
 * 6 Host sends SET_CONFIGURATION ? device is fully enumerated and ready
 */







