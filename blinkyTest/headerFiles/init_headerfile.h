/* 
 * File:   init_headerfile.h
 * Author: thiemen
 *
 * Created on May 13, 2026, 4:29 PM
 */

#ifndef INIT_HEADERFILE_H
#define	INIT_HEADERFILE_H

#ifdef	__cplusplus
extern "C" {
#endif

void timer1setup(void);
void startupPLL(void);
void shutdownPLL(void);
void startupUSB(void);
void shutdownUSB(void);
void setupINEndpoint(void);
void setupOUTEndpoint(void);
void setup0Endpoint(void);
void sendData(void);


typedef struct {
    uint8_t modKeys; //ctrl, alt, shift, windows button (behalve escp)
    uint8_t reserved;
    uint8_t keys[6]; //standaard keyboard 6-key rollover => max. 6 keys tegelijk ingedrukt worden gedetecteerd
}keyType;
#ifdef	__cplusplus
}
#endif

#endif	/* INIT_HEADERFILE_H */

