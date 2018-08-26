/*
 *  user_memory.h - BCU user RAM and user EEPROM.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_user_memory_h
#define sblib_user_memory_h

#include "../types.h"
#include "bcu_type.h"

class UserRam;
class UserEeprom;


/**
 * The user RAM.
 */
extern UserRam& userRam;

/**
 * The user RAM as plain array.
 */
extern byte userRamData[USER_RAM_SIZE+USER_RAM_SHADOW_SIZE];

/**
 * The user EEPROM.
 */
extern UserEeprom& userEeprom;

/**
 * The user EEPROM.
 */
extern byte userEepromData[USER_EEPROM_SIZE];

/**
 * Get a pointer to a user RAM or EEPROM location. This function translates from
 * BCU addresses to native addresses.
 *
 * @param addr - the 16bit BCU address into user RAM or EEPROM.
 * @return A pointer to the memory location.
 */
byte* userMemoryPtr(int addr);


/**
 * The user RAM.
 *
 * The user RAM can be accessed by name, like userRam.status and as an array, like
 * userRam[addr]. Please note that the start address of the RAM is subtracted.
 */
class UserRam
{
public:
    /**
     * 0x0000: Application program data.
     */
	volatile byte user1[0x60];

    /**
     * 0x0060: BCU1 system status. See enum BcuStatus below.
     *         In some modes (BCU2 as BCU1) this part of the RAM
     *         is sued for com objects as well. Therefor the real
     *         status is at the end of the user ram.
     */
	volatile byte _status;

    /**
     * 0x0061: BCU2 application run state.
     *
     * 0 = the application program is halted
     * 1 = the program is running
     * 2 = the program is ready but not running
     * 3 = the program is terminated
     *
     *         In some modes (BCU2 as BCU1) this part of the RAM
     *         is sued for com objects as well. Therefor the real
     *         runState is at the end of the user ram.
     */
	volatile byte _runState;

    /**
     * 0x0062: BCU2 Device control, see enum DeviceControl below.
     *
     * Bit 0: set if the application program is stopped.
     * Bit 1: a telegram with our own physical address was received.
     * Bit 2: send a memory-response telegram automatically on memory-write.
     */
	volatile byte deviceControl;

    /**
     * 0x0063: PEI type. This is the type of the physical external interface
     *         that is connected to the device. It is not used in Selfbus programs.
     */
	volatile byte peiType;

    /**
     * 0x0064: Reserved for system software.
     */
	volatile byte reserved[0x64];

    /**
     * 0x00C8: Application program data.
     */
	volatile byte user2[USER_RAM_SIZE - 0xc8];

    volatile byte status; // real status
    volatile byte runState;

    /**
     * Access the user RAM like an ordinary array. The start address is subtracted
     * when accessing the RAM.
     *
     * @param idx - the index of the data byte to access.
     * @return The data byte.
     */
    byte& operator[](int idx) const;
};


/**
 * The user EEPROM.
 *
 * The user EEPROM can be accessed by name, like userEeprom.status and as an array, like
 * userEeprom[addr]. Please note that the start address of the EEPROM is subtracted. That means
 * userEeprom[0x107] is the correct address for userEeprom.version; not userEeprom[0x07].
 */
class UserEeprom
{
public:
    byte optionReg;      //!< 0x0100: EEPROM option register
    byte manuDataH;      //!< 0x0101: Manufacturing data high byte
    byte manuDataL;      //!< 0x0102: Manufacturing data low byte
    byte manufacturerH;  //!< 0x0103: Software manufacturer high byte
    byte manufacturerL;  //!< 0x0104: Software manufacturer low byte
    byte deviceTypeH;    //!< 0x0105: Device type high byte
    byte deviceTypeL;    //!< 0x0106: Device type low byte
    byte version;        //!< 0x0107: Software version
    byte checkLimit;     //!< 0x0108: EEPROM check limit
    byte appPeiType;     //!< 0x0109: PEI type that the application program requires
    byte syncRate;       //!< 0x010a: Baud rate for serial synchronous PEI
    byte portCDDR;       //!< 0x010b: Port C DDR settings (PEI type 17)
    byte portADDR;       //!< 0x010c: Port A DDR settings
    byte runError;       //!< 0x010d: Runtime error flags
    byte routeCnt;       //!< 0x010e: Routing count constant
    byte maxRetransmit;  //!< 0x010f: INAK and BUSY retransmit limit
    byte confDesc;       //!< 0x0110: Configuration descriptor
    byte assocTabPtr;    //!< 0x0111: Low byte of the pointer to association table (BCU1 only)
    byte commsTabPtr;    //!< 0x0112: Low byte of the pointer to communication objects table (BCU1 only)
    byte usrInitPtr;     //!< 0x0113: Low byte of the pointer to user initialization function (BCU1 only)
    byte usrProgPtr;     //!< 0x0114: Low byte of the pointer to user program function (BCU1 only)
#if BCU_TYPE == BCU1_TYPE
    byte usrSavePtr;     //!< 0x0115: Low byte of the pointer to user save function (BCU1 only)
    byte addrTabSize;    //!< 0x0116: Size of the address table
    byte addrTab[2];     //!< 0x0117+: Address table, 2 bytes per entry. Real array size is addrTabSize*2
    byte user[220];      //!< 0x0116: User EEPROM: 220 bytes (BCU1)
    byte checksum;       //!< 0x01ff: EEPROM checksum (BCU1 only)
#else
    byte appType;        //!< 0x0115: Application program type: 0=BCU2, else BCU1
    byte addrTabSize;    //!< 0x0116: Size of the address table
    byte addrTab[2];     //!< 0x0117+:Address table, 2 bytes per entry. Real array size is addrTabSize*2
    byte user[855];      //!< 0x0119+:User EEPROM: 856 bytes (BCU2)
                         //!< ------  System EEPROM below (BCU2)
    byte loadState[8];   //!< 0x0470: Load state of the system interface objects
    word addrTabAddr;    //!< 0x0478: Address of the address table
    word assocTabAddr;   //!< 0x047a: Address of the association table
    word commsTabAddr;   //!< 0x047c: Address of the communication object table
    word commsSeg0Addr;  //!< 0x047e: Address of communication object memory segment 0
    word commsSeg1Addr;  //!< 0x0480: Address of communication object memory segment 1
    word eibObjAddr;     //!< 0x047c: Address of the application program EIB objects, 0 if unused.
    byte eibObjCount;    //!< 0x047e: Number of application program EIB objects.
    byte padding1;       //!< 0x047f: Padding
    word serviceControl; //!< 0x0480: Service control
    word padding2;       //!< 0x0482: Padding
    byte serial[6];      //!< 0x0484: Hardware serial number (4 byte aligned)
    byte order[10];      //!< 0x048a: Ordering information
#endif /*BCU_TYPE*/

    /**
     * Access the user EEPROM like an ordinary array. The start address is subtracted
     * when accessing the EEPROM. So use userEeprom[0x107] to access userEeprom.version.
     *
     * @param idx - the index of the data byte to access.
     * @return The data byte.
     */
    byte& operator[](int idx) const;
    unsigned char getUInt8(int idx) const;

    /**
     * Access the user EEPROM like an ordinary array. The start address is subtracted
     * when accessing the EEPROM. So use userEeprom[0x107] to access userEeprom.version.
     *
     * @param idx - the index of the 16 bit data to access.
     * @return The 16bit as unsigned int.
     */
    unsigned short getUInt16(int idx) const;

    /**
     * Mark the user EEPROM as modified. The EEPROM will be written to flash when the
     * bus is idle, all telegrams are processed, and no direct data connection is open.
     */
    void modified();

    /**
     * Test if the user EEPROM is modified.
     */
    bool isModified() const;
};


/**
 * BCU status bits for userRam.status (at 0x0060).
 * See BCU1 / BCU2 help for detailed description.
 */
enum BcuStatus
{
    BCU_STATUS_PROG= 0x01,    //!< BCU status: programming mode: 0=normal mode, 1=programming mode
    BCU_STATUS_LL  = 0x02,    //!< BCU status: link layer mode (1), or bus monitor mode (0)
    BCU_STATUS_TL  = 0x04,    //!< BCU status; transport layer enabled
    BCU_STATUS_AL  = 0x08,    //!< BCU status: application layer enabled
    BCU_STATUS_SER = 0x10,    //!< BCU status: serial PEI enabled
    BCU_STATUS_USR = 0x20,    //!< BCU status: application program enabled
    BCU_STATUS_DWN = 0x40,    //!< BCU status: download mode enabled
    BCU_STATUS_PARITY = 0x80  //!< BCU status: parity bit: even parity for bits 0..6)
};


/**
 * Device control flags, for userRam.deviceControl
 */
enum DeviceControl
{
    DEVCTRL_APP_STOPPED = 0x01,      //!< the application program is stopped.
    DEVCTRL_OWN_ADDR_IN_USE = 0x02,  //!< a telegram with our own physical address was received.
    DEVCTRL_MEM_AUTO_RESPONSE = 0x04 //!< send a memory-response telegram automatically on memory-write.
};



//
//  Inline functions
//

inline byte& UserRam::operator[](int idx) const
{
    extern int userRamStart;

    return *(((byte*) this) + idx - userRamStart);
}

inline byte& UserEeprom::operator[](int idx) const
{
    return *(((byte*) this) + idx - USER_EEPROM_START);
}

inline unsigned char UserEeprom::getUInt8(int idx) const
{
    return *(((unsigned char*) this) + idx - USER_EEPROM_START);
}

inline unsigned short UserEeprom::getUInt16(int idx) const
{
    byte * addr = (((byte*) this) + idx - USER_EEPROM_START);
    return (*addr << 8) | *(addr + 1);
}

inline void UserEeprom::modified()
{
    extern byte userEepromModified;
    extern unsigned int writeUserEepromTime;

    userEepromModified = 1;
    writeUserEepromTime = 0;
}

inline bool UserEeprom::isModified() const
{
    extern byte userEepromModified;
    return userEepromModified;
}

inline byte* userMemoryPtr(int addr)
{
    extern int userRamStart;

    if (addr >= USER_EEPROM_START && addr < USER_EEPROM_END)
        return userEepromData + (addr - USER_EEPROM_START);
    else if (addr >= userRamStart && addr < (userRamStart + USER_RAM_SIZE))
        return userRamData + (addr - userRamStart);
    return 0;
}

inline void setUserRamStart(int addr)
{
    extern int userRamStart;

    userRamStart = addr;
}

inline int getUserRamStart(void)
{
    extern int userRamStart;

    return userRamStart;
}

#endif /*sblib_user_memory_h*/
