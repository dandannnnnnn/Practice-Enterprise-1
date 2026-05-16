/* 
 * File:   bluetooth.c
 * Author: Danie
 *
 * Created on May 14, 2026, 2:27 PM
 */

/*
 * Datasheet ble module: https://www.feasycom.com/datasheet/fsc-bt836.pdf
 * Document AT-commands ble module: https://www.manualslib.com/manual/2033834/Feasycom-Fsc-Blueware.html?page=7#manual
 * 
 * formatering command: AT <command> {=Parameter1{,parameter2{,...}}} <CR><LF>
 * 
 * <CR> (carriage return) ==> 0x0D ==> in C-code: '\r'
 * <LF> (line feed) ==> 0x0A ==> C-code: '\n'
 * 
 * "OK" = succes als resultaat
 * "Err<code>" = failure als resultaat
 * 
 * Err001 = failed
 * Err002 = invalid parimeter
 * Err003 = invalid status
 * Err004 = command mismatch
 * 
 * Err005 = busy
 * Err006 = not supported
 * Err007 = No memory
 * 
 * voorbeeld code (local name lezen van ble): AT+NAME ==> Feasycom
 * Test code om te zien dat verbinding (TX atmega --> RX ble module) werkt: 'AT' ==> response = 'OK'
 * 
 * MAC-ADDR lezen van BLE: 'AT+LEADDR'
 * 
 * AT+PROFILE=512 ==> 512 komt van bit 9 dat is voor HID Keyboard (at-command datasheet p.9)
 */

#include <avr/io.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "bluetooth.h"
#include "keys.h"

#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define ubrrVALUE ((F_CPU/(16*9600)) -1) //berekening komt van datasheet atmega32U4 p.191

#define ROWS 7 //input
#define COLS 9 //output

typedef struct {
    uint8_t modKeys; //ctrl, alt, shift, windows button (behalve escp)
    uint8_t reserved;
    uint8_t keys[6]; //standaard keyboard 6-key rollover => max. 6 keys tegelijk ingedrukt worden gedetecteerd
}keyType;

keyType globalReport;
keyType prevReport; 

uint8_t keyMap[ROWS][COLS] = {
    {HID_KEY_ESCAPE, HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE},
    {HID_KEY_TAB, HID_KEY_A, HID_KEY_Z, HID_KEY_E, HID_KEY_R, HID_KEY_T, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE},
    {HID_KEY_LeftControl, HID_KEY_Q, HID_KEY_S, HID_KEY_D, HID_KEY_F, HID_KEY_G, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE},
    {HID_KEY_LeftShift, HID_KEY_W, HID_KEY_X, HID_KEY_C, HID_KEY_V, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE}
};

//==================================
//===============UART===============
//==================================

void UART_Init(void) {
    UBRR1H = (unsigned char)(ubrrVALUE >> 8);
    UBRR1L = (unsigned char)(ubrrVALUE);
    uart1_init();
}

void uart_send(char c) {
    while (!(UCSR1B & (1 << UDRE1))); //wachten tot buffer is leeg
    UDR1 = c;
}

void uart_sendStr(const char* str) {
    while(*str) {
        uart_send(*str++);
    }
}

void sendHEX(uint8_t value) { //zet HEXcode om naar leesbare data voor ble module
    const char hexCHARS[] = "0123456789ABCDEF"; //alle mogelijke hex combinaties
    uart_send(hexCHARS[(value >> 4) & 0x0F]); //range moet binnen de 0-15 blijven
    uart_send(hexCHARS[value & 0x0F]); //alles weg filteren behalve laatste 4 bits                                                                                                                                                                                                                                                                                                                                                                                    
}

//==================================
//===============HID BLE=============
//==================================

void bleReport(void) { //geheugen efficiënt gebruiken ==> enkel veranderingen detecteren & versturen
    if (memcmp(&globalReport, &prevReport, sizeof(keyType)) ==0) {
        return; //huidig key inputs met vorige controleren ==> geen veranderingen dan stopt bleReport()
    }
    
    uart_sendStr("AT+HIDSEND=0x01,"); //0x01 detectie voor toetsenbord
    sendHEX(globalReport.modKeys);
    sendHEX(0x00); //aanvulling ==> verplicht om officiële HID formaat te volgen (byte 0 = modKeys ; byte 1 = reserved (0x00) ; byte 2-7 = keys)
    
    for (uint8_t i = 0; i < 6; i++) {
        sendHEX(globalReport.keys[i]); 
    }
    uart_sendStr("\r\n"); //einde verstuurd msg
    memcpy(&prevReport, &globalReport, sizeof(keyType)); //huidige key inputs opgeslaan in prevReport
}

//==================================
//===============MATRIX===============
//==================================

void updateKeyType(uint8_t key, uint8_t pressedKey) {
    if (key == 0) { //geen toets dan stopt updateKeyType()
        return;
    }
    
    if (key >= 0xE0 && key <= 0xE7) { //modKey ja? nee?
        if (pressedKey) {
            globalReport.modKeys |= (1 << (key - 0xE0)); //modKey gedetecteerd ==> modKey byte op 1
        } else {
            globalReport.modKeys &= ~(1 << (key - 0xE0)); //geen detectie ==> modKey byte op 0
        }
    } else { //normale keys
        if (pressedKey) {
            for (uint8_t i=0 ; i < 6; i++) {
                if (globalReport.keys[i] == key) { //key al in lijst, er gebeurt niks
                    return;
                }
                if (globalReport.keys[i] == 0) {
                    globalReport.keys[i] = key; //eerste lege plaats zoeken en daar key code bewaren
                    return;
                }
            }
        } else{ //key losgelaten
            for (uint8_t i=0; i<6; i++) {
                if (globalReport.keys[i] == key) { //losgelaten key zoeken en verwijderen uit lijst
                    globalReport.keys[i] = 0;
                }
            }
        }
    }
}

void setupCols(uint8_t col, uint8_t status) {
    if (status) { //staus = 1? ==> kolom-pin = HIGH
        if (col == 0) {
            C1_PORT |= (1<<C1_BIT);
        } else if (col == 1) {
            C2_PORT |= (1 << C2_BIT);
        }else if (col == 2) {
            C3_PORT |= (1 << C3_BIT);
        }else if (col == 3) {
            C4_PORT |= (1 << C4_BIT);
        }else if (col == 4) {
            C5_PORT |= (1 << C5_BIT);
        }else if (col == 5) {
            C6_PORT |= (1 << C6_BIT);
        }else if (col == 6) {
            C7_PORT |= (1 << C7_BIT);
        }else if (col == 7) {
            C8_PORT |= (1 << C8_BIT);
        }else if (col == 8) {
            C9_PORT |= (1 << C9_BIT);
        }
    } else { //status = 0 ? ==> kolom-pin = LOW
        if (col == 0) {
            C1_PORT &= ~(1<<C1_BIT);
        } else if (col == 1) {
            C2_PORT &= ~(1 << C2_BIT);
        }else if (col == 2) {
            C3_PORT &= ~(1 << C3_BIT);
        }else if (col == 3) {
            C4_PORT &= ~(1 << C4_BIT);
        }else if (col == 4) {
            C5_PORT &= ~(1 << C5_BIT);
        }else if (col == 5) {
            C6_PORT &= ~(1 << C6_BIT);
        }else if (col == 6) {
            C7_PORT &= ~(1 << C7_BIT);
        }else if (col == 7) {
            C8_PORT &= ~(1 << C8_BIT);
        }else if (col == 8) {
            C9_PORT &= ~(1 << C9_BIT);
        }
    }
}


void matrixScan(void) {
    for (uint8_t c = 0; c < COLS; c++) {
        setupCols(c,1); //kolom = HIGH
        timer1_interrupt(1); //1ms delay
        
        updateKeyType(keyMap[0][c], (R1_PIN & (1 <<R1_BIT)));
        updateKeyType(keyMap[1][c], (R2_PIN & (1 <<R2_BIT)));
        updateKeyType(keyMap[2][c], (R3_PIN & (1 <<R3_BIT)));
        updateKeyType(keyMap[3][c], (R4_PIN & (1 <<R4_BIT)));
        updateKeyType(keyMap[4][c], (R5_PIN & (1 <<R5_BIT)));
        updateKeyType(keyMap[5][c], (R6_PIN & (1 <<R6_BIT)));
        updateKeyType(keyMap[6][c], (R7_PIN & (1 <<R7_BIT)));
        
        setupCols(c,0); //kolom = LOW
    }
}


int main(void) {
    
    cols(); //keys.h
    rows(); //keys.h
    
    for (uint8_t i=0; i<COLS; i++){
        setupCols(i,0); //beginnen met kolom = LOW
    }
    
    UART_Init();
    timer1_interrupt(1000); //1sec delay
    
    uart_sendStr("AT\r\n"); //om te controleren of verbinding succesvol is
    timer1_interrupt(500);
    uart_sendStr("AT+PROFILE=512\r\n"); //selectie HID keyboard
    timer1_interrupt(500);
    uart_sendStr("AT+LENAME=SplitKeyboard\r\n"); //identificatie naam voor bleutooth
    timer1_interrupt(500);
    
    uart_sendStr("AT+RESET\r\n"); //onthoudt de bovenste instellingen
    
    while(1) {
        matrixScan();
        bleReport();
    }
}
