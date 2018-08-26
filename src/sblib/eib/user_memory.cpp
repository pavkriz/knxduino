/*
 *  user_memory.cpp - BCU user RAM and user EEPROM.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "user_memory.h"

//#include <sblib/internal/iap.h>
#include "bus.h"
//#include <sblib/core.h>

#include <string.h>

// move userRamData to a nice address (ok it's stupid but it helps debugging)
byte __attribute__ ((aligned (4))) userRamPadding[20];

byte  __attribute__ ((aligned (4))) userRamData[USER_RAM_SIZE+USER_RAM_SHADOW_SIZE];
UserRam& userRam = *(UserRam*) userRamData;
// TODO: the line above causes a compiler warning which should be avoided:
// warning : dereferencing type-punned pointer will break strict-aliasing rules [-Wstrict-aliasing]

byte  __attribute__ ((aligned (4))) userEepromData[USER_EEPROM_SIZE];
UserEeprom& userEeprom = *(UserEeprom*) userEepromData;
// TODO: the line above causes a compiler warning which should be avoided:
// warning : dereferencing type-punned pointer will break strict-aliasing rules [-Wstrict-aliasing]

volatile byte userEepromModified;
volatile unsigned int writeUserEepromTime;
int userRamStart = USER_RAM_START_DEFAULT;

#define NUM_EEPROM_PAGES     (FLASH_SECTOR_SIZE / USER_EEPROM_FLASH_SIZE)
#define FLASH_SECTOR_ADDRESS (FLASH_BASE_ADDRESS + iapFlashSize() - FLASH_SECTOR_SIZE)
#define LAST_EEPROM_PAGE     (FLASH_SECTOR_ADDRESS + USER_EEPROM_FLASH_SIZE * (NUM_EEPROM_PAGES - 1))

/*
 * Find the last valid page in the flash sector
 */
byte* findValidPage()
{
    // byte* firstPage = FLASH_BASE_ADDRESS + iapFlashSize() - FLASH_SECTOR_SIZE;
    // byte* page = LAST_EEPROM_PAGE;

    // while (page >= firstPage)
    // {
    //     if (page[USER_EEPROM_SIZE - 1] != 0xff)
    //         return page;

    //     page -= USER_EEPROM_FLASH_SIZE;
    // }

    return 0;
}

void readUserEeprom()
{
    byte* page = findValidPage();

    if (page) memcpy(userEepromData, page, USER_EEPROM_SIZE);
    else memset(userEepromData, 0, USER_EEPROM_SIZE);

    userEepromModified = false;
}


void writeUserEeprom()
{
//     if (!userEepromModified)
//         return;

//     // Wait for an idle bus and then disable the interrupts
//     while (!bus.idle())
//         ;
//     noInterrupts();

//     byte* page = findValidPage();
//     if (page == LAST_EEPROM_PAGE)
//     {
//         // Erase the sector
//         int sectorId = iapSectorOfAddress(FLASH_SECTOR_ADDRESS);
//         IAP_Status rc = iapEraseSector(sectorId);
//         if (rc != IAP_SUCCESS) fatalError(); // erasing failed

//         page = FLASH_SECTOR_ADDRESS;
//     }
//     else if (page)
//         page += USER_EEPROM_FLASH_SIZE;
//     else page = FLASH_SECTOR_ADDRESS;

//     userEepromData[USER_EEPROM_SIZE - 1] = 0; // mark the page as in use

//     IAP_Status rc;

// #if (USER_EEPROM_SIZE == 2048) || (USER_EEPROM_SIZE == 3072)
//     rc = iapProgram(page, userEepromData, 1024);
//     if (rc == IAP_SUCCESS)
//         rc = iapProgram(page + 1024, userEepromData + 1024, 1024);
// #if USER_EEPROM_SIZE == 3072
//     if (rc == IAP_SUCCESS)
//         rc = iapProgram(page + 2048, userEepromData + 2048, 1024);
// #endif
// #else
//     rc = iapProgram(page, userEepromData, USER_EEPROM_SIZE);
// #endif
//     if (rc != IAP_SUCCESS) fatalError(); // flashing failed

//     interrupts();
//     userEepromModified = 0;
}
