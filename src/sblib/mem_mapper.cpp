/*
 * mem_mapper.cpp
 *
 *  Created on: Aug 16, 2015
 *      Author: Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 *
 */

//#include <sblib/platform.h>
//#include <sblib/internal/iap.h>
#include "utils.h"
#include "mem_mapper.h"
#include <string.h>
#include <sys/param.h>


MemMapper::MemMapper(unsigned int flashBase, unsigned int flashSize, bool autoAddPage) :
        flashBase(flashBase), flashSize(flashSize), autoAddPage(autoAddPage)
{
    flashSizePages = flashSize / FLASH_PAGE_SIZE;
    flashBasePage = ((unsigned int) flashBase) / FLASH_PAGE_SIZE;
    lastAllocated = 0; // means: nothing allocated in this run
    writePage = 0;
    allocTableModified = false;
    flashMemModified = false;
    memcpy(allocTable, (byte *)flashBase, FLASH_PAGE_SIZE);
}

int MemMapper::doFlash(void) const
{
    int ret = 0;
    // TODO
    // if (allocTableModified)
    // {
    //     if (iapErasePage(flashBasePage) != IAP_SUCCESS)
    //     {
    //         fatalError();
    //     }
    //     if (iapProgram((byte *)flashBase, allocTable, FLASH_PAGE_SIZE) != IAP_SUCCESS)
    //     {
    //         fatalError();
    //     }
    //     allocTableModified = false;
    //     ret |= 1;
    // }
    // if (flashMemModified)
    // {
    //     if (iapErasePage(writePage) != IAP_SUCCESS)
    //     {
    //         fatalError();
    //     }
    //     if (iapProgram((byte *) (writePage << 8), writeBuf, FLASH_PAGE_SIZE)
    //             != IAP_SUCCESS)
    //     {
    //         fatalError();
    //     }
    //     flashMemModified = false;
    //     ret |= 2;
    // }
    return ret;
}

int MemMapper::allocatePage(int virtPage)
{
    if (lastAllocated == 0)
    { // not yet found the highest used entry
        for (int i = 0; i < FLASH_PAGE_SIZE; i++)
        {
            unsigned int entry = allocTable[i] ^ 0xff;
            if (entry > lastAllocated)
            {
                lastAllocated = entry;
            }
        }
    }
    if (lastAllocated == (flashBasePage + flashSizePages - 1))
    {
        return MEM_MAPPER_OUT_OF_MEMORY; // we are out of memory
    }
    if (lastAllocated == 0)
    {  // no pages allocated yet.
        writePage = flashBasePage + 1;
    } else
    {
        lastAllocated++;
        writePage = lastAllocated;
    }
    memset(writeBuf, 0, FLASH_PAGE_SIZE);

    allocTable[virtPage] = writePage ^ 0xff;
    return MEM_MAPPER_SUCCESS;
}

int MemMapper::addRange(int virtAddress, int length)
{
    bool tableModified = false;
    int virtPage = virtAddress >> 8;

    if ((virtAddress & 0xff) || virtPage < 0 || virtPage >= FLASH_PAGE_SIZE)
    {
        return MEM_MAPPER_INVALID_ADDRESS;
    }

    if ((length & 0xff) != 0)
    {
        return MEM_MAPPER_INVALID_LENGTH;
    }

    byte pages = length >> 8;

    for (int page = virtPage; page < (pages + virtPage); page++)
    {
        byte flashPageNum = allocTable[page] ^ 0xff;
        if (flashPageNum == 0)
        { // not yet allocated in flash memory
            int result = allocatePage(page);
            if (result != MEM_MAPPER_SUCCESS)
            {
                return result;
            }
            flashMemModified = true;
            doFlash();
            tableModified = true;
        }
    }
    if (tableModified)
        allocTableModified = true;
    doFlash();
    return MEM_MAPPER_SUCCESS;
}

int MemMapper::getFlashPageNum(int virtAddress) const
{
    int virtPage = virtAddress >> 8;

    if (virtPage < 0 || virtPage >= FLASH_PAGE_SIZE)
    {
        return MEM_MAPPER_INVALID_ADDRESS;
    }

    return allocTable[virtPage] ^ 0xff;
}

int MemMapper::writeMem(int virtAddress, byte data)
{
    int flashPageNum = getFlashPageNum(virtAddress);
    if (flashPageNum < 0)
    {
        return flashPageNum;
    }
    if (writePage != flashPageNum)
    {
        doFlash();
        writePage = flashPageNum;
        if (writePage != 0)
        { // swap flash page into write buffer
            memcpy(writeBuf, (byte *)(writePage << 8), FLASH_PAGE_SIZE);
        }
    }

    if (flashPageNum == 0)
    { // not yet allocated in flash memory
        if (autoAddPage)
        {
            int result = allocatePage(virtAddress >> 8);
            if (result != MEM_MAPPER_SUCCESS)
            {
                return result;
            }
            allocTableModified = true;
        }
    }
    writeBuf[(virtAddress & 0xff)] = data;
    flashMemModified = true;

    return MEM_MAPPER_SUCCESS;
}

int MemMapper::writeMemPtr(int virtAddress, byte *data, int length)
{
    for (int i = 0; i < length; i++)
    {
        int result;
        result = writeMem(virtAddress + i, data[i]);
        if (result != MEM_MAPPER_SUCCESS)
        {
            return result;
        }
    }
    return MEM_MAPPER_SUCCESS;
}

int MemMapper::readMem(int virtAddress, byte &data, bool forceFlash)
{
    int flashPageNum = getFlashPageNum(virtAddress);

    if (flashPageNum < 0)
    {
        data = 0x00;
        return flashPageNum;
    }
    if (forceFlash)
    {
        doFlash();
    }
    if (flashPageNum == 0)
    {
        data = 0x00;
        return MEM_MAPPER_NOT_MAPPED;
    } else if ((flashPageNum == writePage) && !forceFlash)
    {
        data = writeBuf[virtAddress & 0xff];
    } else
    {
        data = ((byte*) (flashPageNum << 8))[virtAddress & 0xff];
    }
    return MEM_MAPPER_SUCCESS;
}

int MemMapper::readMemPtr(int virtAddress, byte *data, int length,
        bool forceFlash)
{
    for (int i = 0; i < length; i++)
    {
        int result;
        result = readMem(virtAddress + i, data[i], forceFlash);
        if (result != MEM_MAPPER_SUCCESS)
        {
            return result;
        }
    }
    return MEM_MAPPER_SUCCESS;
}

bool MemMapper::isMapped(int virtAddress)
{
    return getFlashPageNum(virtAddress) > 0 || autoAddPage ? true : false;
}

byte* MemMapper::memoryPtr(int virtAddress, bool forceFlash) const
{
    int flashPageNum = getFlashPageNum(virtAddress);

    if (flashPageNum < 0)
    {
        return NULL;
    }
    if (forceFlash)
    {
        doFlash();
    }
    if (flashPageNum == 0)
    {
        return NULL;
    } else if ((flashPageNum == writePage) && !forceFlash)
    {
        return writeBuf + (virtAddress & 0xff);
    }
    return ((byte*) (flashPageNum << 8) + (virtAddress & 0xff));
}

 unsigned char MemMapper::getUInt8(int virtAddress)
{
    byte ret;
    readMem(virtAddress, ret);
    return ret;
}

unsigned char& MemMapper::operator[] (const int nIndex) const
{
    return memoryPtr(nIndex)[0];
}

unsigned int MemMapper::getUIntX(int virtAddress, int length)
{
	unsigned int ret = 0;
	int address;

	for(int i = 0; i < length; i++)
	{
		byte b;
		if(endianess == BIG_ENDIAN)
			address = virtAddress + i;
		else
			address = virtAddress + length - i - 1;
		readMem( address , b);
		ret <<= 8;
		ret |= (unsigned int)b;
	}
    return ret;
}

unsigned short MemMapper::getUInt16(int virtAddress)
{
	return (unsigned short)getUIntX(virtAddress, 2);
}

unsigned int MemMapper::getUInt32(int virtAddress)
{
	return (unsigned int)getUIntX(virtAddress, 4);
}

int MemMapper::setUInt8(int virtAddress, byte data)
{
    return writeMem(virtAddress, data);
}

int MemMapper::setUIntX(int virtAddress, int length, int val)
{
	unsigned int ret = 0;
	int address;
	for(int i = 0; i < length; i++)
	{
		if(endianess == BIG_ENDIAN)
			address = virtAddress + length - i - 1;
		else
			address = virtAddress + i;
        ret |= writeMem(address, val & 0xff);
        val >>= 8;
	}
    return ret;
}

int MemMapper::setUInt16(int virtAddress, unsigned short data)
{
	return setUIntX(virtAddress, 2, data);
}

int MemMapper::setUInt32(int virtAddress, unsigned int data)
{
	return setUIntX(virtAddress, 4, data);
}

