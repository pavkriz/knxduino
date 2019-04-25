/*
 * boot_descriptor_block.c
 *
 *  Created on: 10.07.2015
 *      Author: glueck
 */

#include "boot_descriptor_block.h"
#include "crc.h"

#if 1
unsigned int checkVectorTable(unsigned int start)
{
    unsigned int i;
    unsigned int * address;
    unsigned int cs = 0;
    address = (unsigned int *) start;
    for (i = 0; i < 8; i++, address++)
        cs += *address;
    return cs == 0;
}
#else
#define checkVectorTable(s) 1
#endif

unsigned int checkApplication(AppDescriptionBlock * block)
{
	if (block->startAddress >= 0x08020000)	// flash end
        return 0;
    if (block->startAddress < 0x08004000)	// bootloader end (begin of application's flash region)
        return 0;
    if (block->endAddress >= 0x08020000)		// flash end
        return 0;
    if (block->startAddress >= block->endAddress)
        return 0;

    unsigned int crc = crc32(0xFFFFFFFF, (unsigned char *) block->startAddress,
            block->endAddress - block->startAddress);

    return (crc == block->crc);
    //if (crc == block->crc)
    //{
        //return checkVectorTable(block->startAddress);	// not supported on STM32
    //}
    //return 0;
}

inline unsigned char * getAppVersion(AppDescriptionBlock * block)
{
    return (unsigned char *) (block->appVersionAddress);
}

