/*
 *  addr_tables.cpp - BCU communication address tables.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "addr_tables.h"

#include "property_types.h"
#include "user_memory.h"
//#include <sblib/internal/functions.h>
//#include <sblib/bits.h>

int indexOfAddr(int addr)
{
    byte* tab = addrTable();
    int num = 0;
    if (tab)
        num = *tab;

    int addrHigh = addr >> 8;
    int addrLow = addr & 255;

    tab += 3;
    for (int i = 1; i <= num; ++i, tab += 2)
    {
        if (tab[0] == addrHigh && tab[1] == addrLow)
            return i;
    }

    return -1;
}

int objectOfAddr(int addr)
{
    int addrIndex = indexOfAddr(addr);

    byte* tab = assocTable();
    int num = 0;
    if (tab)
        num = *tab++;

    for (int i = 0; i < num; ++i, tab += 2)
    {
        if (tab[0] == addrIndex)
            return tab[1];
    }

    return -1;
}

int addrForSendObject(int objno)
{
    return 0;
}

byte* addrTable()
{
#if BCU_TYPE == BCU1_TYPE
    return (byte*) &userEeprom.addrTabSize;
#else
    byte * addr = (byte* ) & userEeprom.addrTabAddr;
    return userMemoryPtr (makeWord (*(addr + 1), * addr));
#endif
}

byte* assocTable()
{
#if BCU_TYPE == BCU1_TYPE
    return userEepromData + userEeprom.assocTabPtr;
#else
    byte * addr = (byte* ) & userEeprom.assocTabAddr;
    return userMemoryPtr (makeWord (*(addr + 1), * addr));
#endif
}
