#ifndef _DECOMPRESSOR_H_
#define _DECOMPRESSOR_H_ 1

#include <stdint.h>
#include "../src/platform.h"

/**
 * Apply diff stream and produce one page to be flashed
 * (based on diff stream, original ROM content, and RAM buffer to store some latest ROM pages already flashed)
 */
class Decompressor
{

	private:
		enum class State
		{
			EXPECT_COMMAND_BYTE,
			EXPECT_COMMAND_PARAMS,
			EXPECT_RAW_DATA
		};

	private:
		uint8_t cmdBuffer[5];
		int expectedCmdLength = 0;
		int cmdBufferLength = 0;
		uint8_t scratchpad[FLASH_PAGE_SIZE];
		uint8_t oldPages[FLASH_PAGE_SIZE * REMEMBER_OLD_PAGES_COUNT];
		int scratchpadIndex = 0;
		int rawLength = 0;
		State state = State::EXPECT_COMMAND_BYTE;
		uint8_t* startAddrOfPageToBeFlashed = 0;
		uint8_t* startAddrOfFlash = 0;

	public:
		Decompressor(int startAddrOfFirstPageToBeFlashed);

	private:
		int getLength();

		bool isCopyFromRam();

		int getCopyAddress();

		void resetStateMachine();

	public:

		void putByte(uint8_t data);

		bool pageCompletedDoFlash();

		uint32_t getCrc32();

		uint32_t getStartAddrOfPageToBeFlashed();

		uint32_t getBytesCountToBeFlashed();

		uint8_t getFlashPageNumberToBeFlashed();

};

#endif
