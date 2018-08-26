/*
 *  utils.cpp - Utility functions.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "utils.h"

//#include <sblib/digital_pin.h>
//#include <sblib/platform.h>


void reverseCopy(byte* dest, const byte* src, int len)
{
    src += len - 1;
    while (len > 0)
    {
        *dest++ = *src--;
        --len;
    }
}

void fatalError()
{
    // // We use only low level functions here as a fatalError() could happen
    // // anywhere in the lib and we want to ensure that the function works
    // pinMode(PIO0_7, OUTPUT);
    // SysTick_Config(0x1000000);

    while (1)
    {
        // Blink the LED on the LPCxpresso board
        //digitalWrite(PIO0_7, (SysTick->VAL & 0x800000) == 0 ? 1 : 0);
    }
}
