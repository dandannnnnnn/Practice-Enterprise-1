/* 
 * File:   USB_TEST_CODE.c
 * Author: stephcuv
 *
 * Created on May 16, 2026, 11:21 PM
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include<avr/interrupt.h>
#include "../headerFiles/init_headerfile.h"


/*
 * 
 */
int main(int argc, char** argv) {
    startupUSB();
    return (EXIT_SUCCESS);
}

