/* 
 * File:   USB_test_code.c
 * Author: stephcuv
 *
 * Created on May 13, 2026, 10:08 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "init_headerfile.h"

/*
 * 
 */
int main(int argc, char** argv) 
{
    setup();
    
    return (EXIT_SUCCESS);
}



//zie 272 datasheet
void addressSetup(void)
{
    return 0;
}


void setup()
{
 startupUSB(void);   
}
