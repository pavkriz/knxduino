/*
 *  timer.h - Timer manipulation and time functions.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_timer_h
#define sblib_timer_h

#include <Arduino.h>

/**
 * Get the number of milliseconds that elapsed since the reference time.
 *
 * @param ref - the reference time to compare with
 * @return The numer of milliseconds since time.
 */
unsigned int elapsed(unsigned int ref);

inline unsigned int elapsed(unsigned int ref)
{
    return millis() - ref;
}

#endif /*sblib_timer_h*/