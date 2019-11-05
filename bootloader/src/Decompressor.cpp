#include "Decompressor.h"

#include <string.h>
#include "stm32g0xx_hal_flash.h"
#include "crc.h"

#define CMD_RAW 0
#define CMD_COPY 0b10000000
#define FLAG_LONG 0b01000000
#define ADDR_FROM_ROM 0
#define ADDR_FROM_RAM 0b10000000

Decompressor::Decompressor(int startAddrOfFirstPageToBeFlashed)
{
	startAddrOfFlash = (uint8_t*)startAddrOfFirstPageToBeFlashed;
	startAddrOfPageToBeFlashed = (uint8_t*)startAddrOfFirstPageToBeFlashed;
}

int Decompressor::getLength()
{
	if ((cmdBuffer[0] & FLAG_LONG) == FLAG_LONG)
	{
		return ((cmdBuffer[0] & 0b111111) << 8) | (cmdBuffer[1] & 0xff);
	}
	else
	{
		return (cmdBuffer[0] & 0b111111);
	}
}

bool Decompressor::isCopyFromRam()
{
	if ((cmdBuffer[0] & FLAG_LONG) == FLAG_LONG)
	{
		return (cmdBuffer[2] & ADDR_FROM_RAM) == ADDR_FROM_RAM;
	}
	else
	{
		return (cmdBuffer[1] & ADDR_FROM_RAM) == ADDR_FROM_RAM;
	}
}

int Decompressor::getCopyAddress()
{
	if ((cmdBuffer[0] & FLAG_LONG) == FLAG_LONG)
	{
		return ((cmdBuffer[2] & 0b1111111) << 16) | ((cmdBuffer[3] & 0xff) << 8) | (cmdBuffer[4] & 0xff);
	}
	else
	{
		return ((cmdBuffer[1] & 0b1111111) << 16) | ((cmdBuffer[2] & 0xff) << 8) | (cmdBuffer[3] & 0xff);
	}
}

void Decompressor::resetStateMachine()
{
	state = State::EXPECT_COMMAND_BYTE;
}

bool Decompressor::pageCompletedDoFlash()
{
	// backup old page content, flash new content of scratchpad to flash
	bool result = true;

	// backup original content of the page to be flashed (rewritten) to RAM:
	// first, shift old data one page back, will leave one "empty" page at the end
	memcpy(oldPages, oldPages + FLASH_PAGE_SIZE, sizeof(oldPages) - FLASH_PAGE_SIZE);
	// then, fill the last (current) page (may not be whole page when EOF) at the end
	memcpy(oldPages + FLASH_PAGE_SIZE * (REMEMBER_OLD_PAGES_COUNT-1), startAddrOfPageToBeFlashed, FLASH_PAGE_SIZE);

	// proceed to flash the decompressed page stored in the scratchpad
	HAL_FLASH_Unlock();
    for (unsigned int i = 0; i < FLASH_PAGE_SIZE; i+=8) {	// 8 = doubleword = 2*32bit = 4*16bit
    	uint64_t wordToWrite = *((uint64_t *) scratchpad + i/8);
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)(startAddrOfPageToBeFlashed + i), wordToWrite) != HAL_OK) {
			result = false; //UPD_FLASH_ERROR;
			break;
		}
    }
	HAL_FLASH_Lock();

	// move to next page
	startAddrOfPageToBeFlashed += FLASH_PAGE_SIZE;
	// reinitialize scratchpad
	scratchpadIndex = 0;
	memset(scratchpad, 0, sizeof scratchpad);
	return result;
}

void Decompressor::putByte(uint8_t data)
{
	//System.out.println("Decompressor processing new byte " + (data & 0xff) + ", state=" + state);
	switch (state)
	{
		case State::EXPECT_COMMAND_BYTE:
			cmdBuffer[0] = data;
			cmdBufferLength = 1;
			expectedCmdLength = 1;
			if ((data & CMD_COPY) == CMD_COPY)
			{
				expectedCmdLength += 3; // 3 more bytes of source address
			}
			if ((data & FLAG_LONG) == FLAG_LONG)
			{
				expectedCmdLength += 1; // 1 more byte for longer length
			}
			if (expectedCmdLength > 1)
			{
				state = State::EXPECT_COMMAND_PARAMS;
			}
			else
			{
				state = State::EXPECT_RAW_DATA;
				rawLength = 0;
			}
			break;
		case State::EXPECT_COMMAND_PARAMS:
			cmdBuffer[cmdBufferLength++] = data;
			if (cmdBufferLength >= expectedCmdLength)
			{
				// we have all params of the command
				if ((cmdBuffer[0] & CMD_COPY) == CMD_COPY)
				{
					// perform copy
					if (isCopyFromRam())
					{
						//System.out.println("COPY FROM RAM index=" + scratchpadIndex + " length=" + getLength() + " from addr=" + getCopyAddress());
						//System::arraycopy(oldPagesRam->getOldBinData(), getCopyAddress(), scratchpad, scratchpadIndex, getLength());
						memcpy(scratchpad + scratchpadIndex, oldPages + getCopyAddress(), getLength());
					}
					else
					{
						//System.out.println("COPY FROM ROM index=" + scratchpadIndex + " length=" + getLength() + " from addr=" + getCopyAddress());
						//System.out.println(rom.getBinData()[getCopyAddress()] & 0xff);
						//System.out.println(rom.getBinData()[getCopyAddress()+1] & 0xff);
						//System.out.println(rom.getBinData()[getCopyAddress()+2] & 0xff);
						//System::arraycopy(rom->getBinData(), getCopyAddress(), scratchpad, scratchpadIndex, getLength());
						memcpy(scratchpad + scratchpadIndex, startAddrOfFlash + getCopyAddress(), getLength());
					}
					scratchpadIndex += getLength();
					// and finish command
					resetStateMachine();
				}
				else
				{
					// next, read raw data
					state = State::EXPECT_RAW_DATA;
					rawLength = 0;
				}
			} // else expect more params of the command
			break;
		case State::EXPECT_RAW_DATA:
			// store data read to scratchpad
			scratchpad[scratchpadIndex++] = data;
			rawLength++;
			if (rawLength >= getLength())
			{
				// we have all RAW data, reset state machine
				resetStateMachine();
			}
	}
}

uint32_t Decompressor::getCrc32() {
	return crc32(0xFFFFFFFF, scratchpad, scratchpadIndex);
}

uint32_t Decompressor::getStartAddrOfPageToBeFlashed() {
	return (uint32_t)startAddrOfPageToBeFlashed;
}

uint32_t Decompressor::getBytesCountToBeFlashed() {
	return scratchpadIndex;
}

uint8_t Decompressor::getFlashPageNumberToBeFlashed() {
	return (uint8_t)((uint32_t)startAddrOfPageToBeFlashed / FLASH_PAGE_SIZE);
}
