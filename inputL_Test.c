/*
 * File:   inputL_Test.c
 * Author: Danielle
 *
 * Created on April 24, 2026, 11:27 AM
 */

//keyboard test links

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>
/*#include "inputDetect_header_L.h"*/

#define F_CPU 16000000UL //16MHz xtal
#define ROWS 7
#define COLS 9

typedef struct {
    uint8_t modKeys; //ctrl, alt, shift, windowsbutton, .. (behalve escp)
    uint8_t keys[6];
} keyType;

keyType globalReport;

// ... (Descriptors blijven hetzelfde) ...
const uint8_t PROGMEM device_descriptor[] = {
    18, 1, 0x00, 0x02, 0, 0, 0, 64, 0xEB, 0x03, 0x61, 0x20, 0x00, 0x01, 0, 0, 0, 1
};

const uint8_t PROGMEM configuration_descriptor[] = {
    9, 2, 34, 0, 1, 1, 0, 0xA0, 50,
    9, 4, 0, 0, 1, 3, 1, 1, 0,
    9, 0x21, 0x11, 0x01, 0, 1, 0x22, 45, 0,
    7, 5, 0x81, 3, 8, 0, 10
};

const uint8_t PROGMEM hid_report_descriptor[] = {
    0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x05, 0x07,
    0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00, 0x25, 0x01,
    0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01,
    0x75, 0x08, 0x81, 0x03, 0x95, 0x06, 0x75, 0x08,
    0x15, 0x00, 0x25, 0x65, 0x05, 0x07, 0x19, 0x00,
    0x29, 0x65, 0x81, 0x00, 0xC0
};

uint8_t keyMap[ROWS][COLS] = {
    {HID_KEY_ESCAPE, HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE},
    {HID_KEY_TAB, HID_KEY_A, HID_KEY_Z, HID_KEY_E, HID_KEY_R, HID_KEY_T, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE},
    {HID_KEY_LeftControl, HID_KEY_Q, HID_KEY_S, HID_KEY_D, HID_KEY_F, HID_KEY_G, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE},
    {HID_KEY_LeftShift, HID_KEY_W, HID_KEY_X, HID_KEY_C, HID_KEY_V, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE}
};

void delayUs(uint16_t us) {
    while (us--) {
        asm volatile ("nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t");
    }
}

void delayMs(uint16_t ms) {
    while (ms--) delayUs(1000);
}


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
    PLLCSR = (0 << PINDIV) | (0 << PLLE);
    while ((PLLCSR & (0 << PLOCK)));
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
    USBCON |= (1 << UVREGE); 
    UDCON |= (0 << DETACH);
    
    startupPLL();
    
    USBCON |= (1 << USBE);
    USBCON |= (0 << FRZCLK);
    
    
    UDCON |= (1 << LSM);
}


void shutdownUSB(void) {
    
    UDCON |= (1 << DETACH);
    
    USBCON |= (0 << USBE);
   //USBCON |= (1 << FRZCLK);
    
    shutdownPLL();
    
   
    USBCON |= (0 << UVREGE); 
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
 * type endpoints
 * bulk overzetten grote hoeveelheden data bv file
 * interupt: 
 * isochronous
 * control
 * 
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
 * EPTYPE 7:6 = 00b control 10b bulk, 01b isochronous, 11b interrupt
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

void setupControlEndpoint(void) {
    //endpoint 0
    UENUM = 0;
    //interupt mode aanzetten 
    UECFG0X |= (1 << EPTYPE0);
    UECFG0X |= (1 << EPTYPE1);
    
    //endpoint instellen op IN
    UECFG0X |= (1 << EPDIR);
    
    //endpoint grootte = 8bytes
    UECFG1X = (0 << EPSIZE2) | (0 << EPSIZE1) | (0 << EPSIZE0);
    UECFG1X = (0 << EPBK1) | (0 << EPBK0);
    UECFG1X =(1 << ALLOC);
}
 
void setupEndpoint1(void) {
    UENUM = 1;
    UECONX |= (1 << EPEN);
    UECFG0X = (1 << EPTYPE1) | (1 << EPTYPE0);
    UECFG1X = (1 << EPSIZE0) | (1 << ALLOC);
}

/*----------------------------------------------------------*/

void sendUSBReport(void) {
    UENUM = 1;
    if (!(UEINTX & (1<<RWAL))) return;
    UEDATX = globalReport.modKeys;
    UEDATX = 0;
    for (uint8_t i = 0; i < 6; i++) UEDATX = globalReport.keys[i];
    UEINTX = 0x3A;
}

void usb_task(void) {
    UENUM = 0;
    if (!(UEINTX & (1 << RXSTPE))) return;
    uint8_t bmRequestType = UEDATX;
    uint8_t bRequest = UEDATX;
    uint16_t wValue = UEDATX; wValue |= (UEDATX << 8);
    uint16_t wIndex = UEDATX; wIndex |= (UEDATX << 8);
    uint16_t wLength = UEDATX; wLength |= (UEDATX << 8);
    UEINTX &= ~((1 << RXSTPE));
    if (bRequest == 0x06) {
        const uint8_t* ptr = NULL;
        uint16_t len = 0;
        if (wValue == 0x100) { ptr = device_descriptor; len = sizeof(device_descriptor); }
        else if (wValue == 0x200) { ptr = configuration_descriptor; len = sizeof(configuration_descriptor); }
        else if (wValue == 0x2200) { ptr = hid_report_descriptor; len = sizeof(hid_report_descriptor); }
        if (ptr) {
            uint16_t sendLen = (wLength < len) ? wLength : len;
            while (sendLen > 0) {
                while (!(UEINTX & (1 << TXINE)));
                uint8_t chunk = (sendLen > 64) ? 64 : sendLen;
                for (uint8_t i = 0 ; i < chunk; i++) UEDATX = pgm_read_byte(ptr++);
                sendLen -= chunk;
                UEINTX &= ~(1 << TXINE);
            }
        }
    }
    else if (bRequest == 0x05) {
        UEINTX &= ~(1 << TXINE);
        while (!(UEINTX & (1 << TXINE)));
        UDADDR = (wValue & 0x7F) | (1 << ADDEN);
    }
    else if (bRequest == 0x09) {
        setupEndpoint1();
        UEINTX &= ~(1 << TXINE);
    }
}

void updateKeyType(uint8_t key, uint8_t pressed) {
    if (key == 0) return;
    if (key >= 0xE0 && key <= 0xE7) {
        if (pressed) globalReport.modKeys |= (1 << (key - 0xE0));
        else globalReport.modKeys &= ~(1 << (key - 0xE0));
    } else {
        if (pressed) {
            for (uint8_t i = 0; i < 6; i++) {
                if (globalReport.keys[i] == key) return;
                if (globalReport.keys[i] == 0) { globalReport.keys[i] = key; return; }
            }
        } else {
            for (uint8_t i = 0; i < 6; i++) {
                if (globalReport.keys[i] == key) globalReport.keys[i] = 0;
            }
        }
    }
}

void scanMatrix(void) {
    for (uint8_t r = 0; r < ROWS; r++) {
        PORTF = (1 << r);
        delayUs(15);
        updateKeyType(keyMap[r][0], (PIND & (1<<7)) != 0); //col 1
        updateKeyType(keyMap[r][1], (PINB & (1<<4)) != 0);
        updateKeyType(keyMap[r][2], (PINB & (1<<5)) != 0);
        updateKeyType(keyMap[r][3], (PINB & (1<<6)) != 0);
        updateKeyType(keyMap[r][4], (PINB & (1<<0)) != 0);
        updateKeyType(keyMap[r][5], (PINB & (1<<1)) != 0);
        updateKeyType(keyMap[r][6], (PINB & (1<<2)) != 0);
        updateKeyType(keyMap[r][7], (PINB & (1<<3)) != 0);
        updateKeyType(keyMap[r][8], (PINB & (1<<7)) != 0); //col 9
        PORTF &= ~(1<<r);
    }
}

int main(void) {
    MCUSR &= ~(1 << WDRF);
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = 0;
    
    // Matrix setup
    DDRF = 0x7F;
    PORTF = 0x00;
    DDRB = 0x00;
    PORTB = 0x00;
    DDRD &= ~(1<<7);
    
    // LEDs = output
    DDRD |= (1 << DDD5) | (1 << DDD0);
    // LEDs uit bij startr
    PORTD &= ~((1 << PORTD5) | (1 << PORTD0));
    
    initUSB();
    setupControlEndpoint();
    
    while(1) {
        usb_task();
        
        if (UDADDR & (1 << ADDEN)) {
            scanMatrix();
            sendUSBReport();
            uint8_t keyPressed = 0;
            if (globalReport.modKeys != 0) {
                keyPressed = 1;
            } else {
                for (uint8_t i = 0; i < 6; i++) {
                    if (globalReport.keys[i] != 0) {
                        keyPressed = 1;
                        break;
                    }
                }
            }

            // LED actie
            if (keyPressed) {
                PORTD |= (1 << PORTD5) | (1 << PORTD0); // ON
            } else {
                PORTD &= ~((1 << PORTD5) | (1 << PORTD0)); // OFF
            }

            delayMs(8);
        }
    }
}