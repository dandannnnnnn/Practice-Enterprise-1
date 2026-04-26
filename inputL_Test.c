/*
 * File:   inputL_Test.c
 * Author: Danie
 *
 * Created on April 24, 2026, 11:27 AM
 */

//keyboard test links

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "inputDetect_header_L.h"

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

void setupPLL(void) {
    PLLCSR = (1 << PINDIV) | (1 << PLLE);
    while (!(PLLCSR & (1 << PLOCK)));
}

void initUSB(void) {
    USBCON |= (1 << USBE);
    USBCON &= ~(1 << FRZCLK);
    setupPLL();
    UDCON &= ~(1 << DETACH);
}

void setupControlEndpoint(void) {
    UENUM = 0;
    UECONX |= (1 << EPEN);
    UECFG0X = 0;
    UECFG1X = (1 << EPSIZE1) | (1 << EPSIZE0) | (1 << ALLOC);
}

void setupEndpoint1(void) {
    UENUM = 1;
    UECONX |= (1 << EPEN);
    UECFG0X = (1 << EPTYPE1) | (1 << EPTYPE0);
    UECFG1X = (1 << EPSIZE0) | (1 << ALLOC);
}

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