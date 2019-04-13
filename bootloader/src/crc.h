/*
 *  crc.h - The application's main.
 *
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *                     Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef _CRC32_H_
#define _CRC32_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

unsigned int crc32 (unsigned int start, unsigned char * data, unsigned int count);

#ifdef __cplusplus
}
#endif

#endif
