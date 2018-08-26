/*
 *  com_objects.cpp - EIB Communication objects.
 *
 *  Copyright (C) 2014-2015 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "com_objects.h"

#include "addr_tables.h"
#include "apci.h"
#include "property_types.h"
#include "user_memory.h"
//#include <sblib/internal/functions.h>


// The COMFLAG_UPDATE flag, moved to the high nibble
#define COMFLAG_UPDATE_HIGH (COMFLAG_UPDATE << 4)

// The COMFLAG_TRANS_MASK mask, moved to the high nibble
#define COMFLAG_TRANS_MASK_HIGH (COMFLAG_TRANS_MASK << 4)


// The size of the object types BIT_7...VARDATA in bytes
const byte objectTypeSizes[10] = { 1, 1, 2, 3, 4, 6, 8, 10, 14, 15 };

int le_ptr = BIG_ENDIAN;

int objectSize(int objno)
{
    int type = objectType(objno);
    if (type < BIT_7) return 1;
    return objectTypeSizes[type - BIT_7];
}

/*
 * Get the size of the com-object in bytes, for sending/receiving telegrams.
 * 0 is returned if the object's size is <= 6 bit.
 */
int telegramObjectSize(int objno)
{
    int type = objectType(objno);
    if (type < BIT_7) return 0;
    return objectTypeSizes[type - BIT_7];
}

/*
 * Add one or more flags to the flags of a communication object.
 * This does not clear any flag of the communication object.
 *
 * @param objno - the ID of the communication object
 * @param flags - the flags to add
 *
 * @see objectWritten(int)
 * @see requestObjectRead(int)
 */
void addObjectFlags(int objno, int flags)
{
    byte* flagsTab = objectFlagsTable();
    if(flagsTab == 0)
    	return;

    if (objno & 1)
        flags <<= 4;

    flagsTab[objno >> 1] |= flags;
}

/*
 * Set the flags of a communication object.
 *
 * @param objno - the ID of the communication object
 * @param flags - the new communication object flags
 *
 * @see objectWritten(int)
 * @see requestObjectRead(int)
 */
void setObjectFlags(int objno, int flags)
{
    byte* flagsPtr = objectFlagsTable();
    flagsPtr += objno >> 1;

    if (objno & 1)
    {
        *flagsPtr &= 0x0f;
        *flagsPtr |= flags << 4;
    }
    else
    {
        *flagsPtr &= 0xf0;
        *flagsPtr |= flags;
    }
}

byte* objectValuePtr(int objno)
{
    // The object configuration
    const ComConfig& cfg = objectConfig(objno);

#if BCU_TYPE == BCU1_TYPE
    if (cfg.config & COMCONF_VALUE_TYPE) // 0 if user RAM, >0 if user EEPROM
        return userEepromData + cfg.dataPtr;
    return userRamData + cfg.dataPtr;
#else
    // TODO Should handle userRam.segment0addr and userRam.segment1addr here
    // if (cfg.config & COMCONF_VALUE_TYPE) // 0 if segment 0, !=0 if segment 1
    const byte * addr = (const byte *) &cfg.dataPtr;
    if (le_ptr == LITTLE_ENDIAN)
        return userMemoryPtr(makeWord(addr[1], addr[0]));
    else
        return userMemoryPtr(makeWord(addr[0], addr[1]));
#endif
}

unsigned int objectRead(int objno)
{
	int sz = objectSize(objno);
	byte* ptr = objectValuePtr(objno) + sz;
	unsigned int value = *--ptr;

	while (--sz > 0)
	{
		value <<= 8;
		value |= *--ptr;
	}
    return value;
}

void _objectWrite(int objno, unsigned int value, int flags)
{
    byte* ptr = objectValuePtr(objno);
    int sz = objectSize(objno);

    if(ptr == 0)
        return;

    for (; sz > 0; --sz)
    {
        *ptr++ = value;
        value >>= 8;
    }

    addObjectFlags(objno, flags);
}

void _objectWriteBytes(int objno, byte* value, int flags)
{
    byte* ptr = objectValuePtr(objno);
    int sz = objectSize(objno);

    for (; sz > 0; --sz)
        *ptr++ = *value++;

    addObjectFlags(objno, flags);
}

/*
 * @return The number of communication objects.
 */
inline int objectCount()
{
    // The first byte of the config table contains the number of com-objects
    return *objectConfigTable();
}

/*
 * Find the first group address for the communication object. This is the
 * address that is used when sending a read-value or a write-value telegram.
 *
 * @param objno - the ID of the communication object
 * @return The group address, or 0 if none found.
 */
int firstObjectAddr(int objno)
{
    byte* assocTab = assocTable();
    byte* assocTabEnd = assocTab + (*assocTab << 1);

    for (++assocTab; assocTab < assocTabEnd; assocTab += 2)
    {
        if (assocTab[1] == objno)
        {
            byte* addr = addrTable() + 1 + (assocTab[0] << 1);
            return (addr[0] << 8) | addr[1];
        }
    }

    return 0;
}

/*
 * Create and send a group read request telegram.
 *
 * @param objno - the ID of the communication object
 * @param addr - the group address to read
 */
void sendGroupReadTelegram(int objno, int addr)
{
    bcu.sendTelegram[0] = 0xbc; // Control byte
    // 1+2 contain the sender address, which is set by bus.sendTelegram()
    bcu.sendTelegram[3] = addr >> 8;
    bcu.sendTelegram[4] = addr;
    bcu.sendTelegram[5] = 0xe1;
    bcu.sendTelegram[6] = 0;
    bcu.sendTelegram[7] = 0x00;

    bus.sendTelegram(bcu.sendTelegram, 8);
}

/*
 * Create and send a group write or group response telegram.
 *
 * @param objno - the ID of the communication object
 * @param addr - the destination group address
 * @param isResponse - true if response telegram, false if write telegram
 */
void sendGroupWriteTelegram(int objno, int addr, bool isResponse)
{
    byte* valuePtr = objectValuePtr(objno);
    int sz = telegramObjectSize(objno);

    bcu.sendTelegram[0] = 0xbc; // Control byte
    // 1+2 contain the sender address, which is set by bus.sendTelegram()
    bcu.sendTelegram[3] = addr >> 8;
    bcu.sendTelegram[4] = addr;
    bcu.sendTelegram[5] = 0xe0 | ((sz + 1) & 15);
    bcu.sendTelegram[6] = 0;
    bcu.sendTelegram[7] = isResponse ? 0x40 : 0x80;

    if (sz) reverseCopy(bcu.sendTelegram + 8, valuePtr, sz);
    else bcu.sendTelegram[7] |= *valuePtr & 0x3f;

    // Process this telegram in the receive queue (if there is a local receiver of this group address)
    processGroupTelegram(addr, APCI_GROUP_VALUE_WRITE_PDU, bcu.sendTelegram);

    bus.sendTelegram(bcu.sendTelegram, 8 + sz);
}

int sndStartIdx = 0;

bool sendNextGroupTelegram()
{

    const ComConfig* configTab = &objectConfig(0);
    byte* flagsTab = objectFlagsTable();
    if(flagsTab == 0)
    	return false;

    int addr, flags, objno, config, numObjs = objectCount();

    for (objno = sndStartIdx; objno < numObjs; ++objno)
    {
        flags = flagsTab[objno >> 1];
        if (objno & 1) flags >>= 4;

        if ((flags & COMFLAG_TRANSREQ) == COMFLAG_TRANSREQ)
        {
            unsigned int mask = COMFLAG_TRANS_MASK << (objno & 1 ? 4 :  0);
            flagsTab[objno >> 1] &= ~mask;

            config = configTab[objno].config;
            addr = firstObjectAddr(objno);

            if (addr == 0 || !(config & COMCONF_COMM))
                continue;

            if (flags & COMFLAG_DATAREQ)
                sendGroupReadTelegram(objno, addr);
            else if (config & COMCONF_TRANS)
                sendGroupWriteTelegram(objno, addr, false);
            else continue;

            sndStartIdx = objno + 1;
            return true;
        }
    }

    sndStartIdx = 0;
    return false;
}

int nextUpdatedObject()
{
    static int startIdx = 0;

    byte* flagsTab = objectFlagsTable();
    if(flagsTab == 0)
    	return -1;

    int flags, objno, numObjs = objectCount();

    for (objno = startIdx; objno < numObjs; ++objno)
    {
        flags = flagsTab[objno >> 1];

        if (objno & 1) flags &= COMFLAG_UPDATE_HIGH;
        else flags &= COMFLAG_UPDATE;

        if (flags)
        {
            flagsTab[objno >> 1] &= ~flags;
            startIdx = objno + 1;
            return objno;
        }
    }

    startIdx = 0;
    return -1;
}

void processGroupWriteTelegram(int objno, byte* tel)
{
    byte* valuePtr = objectValuePtr(objno);
    int count = telegramObjectSize(objno);

    if (count > 0) reverseCopy(valuePtr, tel + 8, count);
    else *valuePtr = tel[7] & 0x3f;

    addObjectFlags(objno, COMFLAG_UPDATE);
}

void processGroupTelegram(int addr, int apci, byte* tel)
{
    const ComConfig* configTab = &objectConfig(0);
    const byte* assocTab = assocTable();
    const int endAssoc = 1 + (*assocTab) * 2;
    int objno, objConf;

    // Convert the group address into the index into the group address table
    const int gapos = indexOfAddr(addr);
    if (gapos < 0) return;

    // Loop over all entries in the association table, as one group address
    // could be assigned to multiple com-objects.
    for (int idx = 1; idx < endAssoc; idx += 2)
    {
        // Check if grp-address index in assoc table matches the dest grp address index
        if (gapos == assocTab[idx]) // We found an association for our addr
        {
            objno = assocTab[idx + 1];  // Get the com-object number from the assoc table
            objConf = configTab[objno].config;

            if (apci == APCI_GROUP_VALUE_WRITE_PDU || apci == APCI_GROUP_VALUE_RESPONSE_PDU)
            {
                // Check if communication and write are enabled
                if ((objConf & COMCONF_WRITE_COMM) == COMCONF_WRITE_COMM)
                    processGroupWriteTelegram(objno, tel);
            }
            else if (apci == APCI_GROUP_VALUE_READ_PDU)
            {
                // Check if communication and read are enabled
                if ((objConf & COMCONF_READ_COMM) == COMCONF_READ_COMM)
                    sendGroupWriteTelegram(objno, addr, true);
            }
        }
    }
}

byte* objectConfigTable()
{
#if BCU_TYPE == BCU1_TYPE
    return userEepromData + userEeprom.commsTabPtr;
#else
    byte * addr = (byte* ) & userEeprom.commsTabAddr;
    return userMemoryPtr (makeWord (*(addr + 1), * addr));
#endif
}

byte* objectFlagsTable()
{
#if BCU_TYPE == BCU1_TYPE
    return userRamData + userEepromData[userEeprom.commsTabPtr + 1];
#else
    const byte* configTable = objectConfigTable();
    if(le_ptr == LITTLE_ENDIAN)
    	return userMemoryPtr(makeWord(configTable[2], configTable[1]));

    return userMemoryPtr(makeWord(configTable[1], configTable[2]));
#endif
}
