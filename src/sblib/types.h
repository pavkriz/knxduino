/*
 *  types.h - Data types
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_types_h
#define sblib_types_h

/**
 * An unsigned 1 byte value.
 */
typedef unsigned char byte;

/**
 * An unsigned 2 byte value.
 */
#ifdef IS_BOOTLOADER
typedef unsigned short word;  // is defined in Arduino Wiring
#else
#include <wiring.h>
#endif

/**
 * Declare a function as always inline
 */
#if defined ( __GNUC__ )
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#define ALWAYS_INLINE inline
#endif

#endif /*sblib_types_h*/
