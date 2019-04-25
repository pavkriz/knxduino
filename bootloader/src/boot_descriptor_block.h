/*
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef BOOT_DESCRIPTOR_BLOCK_H_
#define BOOT_DESCRIPTOR_BLOCK_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define FIRST_SECTOR    0x8000
#define BOOT_BLOCK_SIZE  0x800
#define BOOT_BLOCK_PAGE ((FIRST_SECTOR / BOOT_BLOCK_SIZE) - 1)

typedef struct
{
    unsigned int startAddress;
    unsigned int endAddress;
    unsigned int crc;
    unsigned int appVersionAddress;
}__attribute__ ((aligned (BOOT_BLOCK_SIZE))) AppDescriptionBlock;

unsigned int checkApplication(AppDescriptionBlock * block);

unsigned char * getAppVersion(AppDescriptionBlock * block);

#ifdef __cplusplus
}
#endif

#endif /* BOOT_DESCRIPTOR_BLOCK_H_ */
