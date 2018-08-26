/*
 *  types.h - EIB data types
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_eib_types_h
#define sblib_eib_types_h

#include "../types.h"

/**
 * The type of a communication object.
 */
enum ComType
{
    /** Communication object type: 1 bit */
    BIT_1 = 0,

    /** Communication object type: 2 bit */
    BIT_2 = 1,

    /** Communication object type: 3 bit */
    BIT_3 = 2,

    /** Communication object type: 4 bit */
    BIT_4 = 3,

    /** Communication object type: 5 bit */
    BIT_5 = 4,

    /** Communication object type: 6 bit */
    BIT_6 = 5,

    /** Communication object type: 7 bit */
    BIT_7 = 6,

    /** Communication object type: 1 byte */
    BYTE_1 = 7,

    /** Communication object type: 2 bytes */
    BYTE_2 = 8,

    /** Communication object type: 3 bytes */
    BYTE_3 = 9,

    /** Communication object type: 4 bytes / float */
    BYTE_4 = 10,

    /** Communication object type: 4 bytes / float */
    FLOAT = 10,

    /** Communication object type: 6 bytes */
    DATA_6 = 11,

    /** Communication object type: 8 bytes / double */
    DATA_8 = 12,

    /** Communication object type: 8 bytes / double */
    DOUBLE = 12,

    /** Communication object type: 10 bytes */
    DATA_10 = 13,

    /** Communication object type: 14 bytes */
    MAXDATA = 14,

    /** Communication object type: variable length 1-14 bytes */
    VARDATA = 15
};

/**
 * The flags for a communication object configuration.
 */
enum ComConfigFlag
{
    /** Com object configuration flag: transmit enabled */
    COMCONF_TRANS = 0x40,

    /**
     * Com object configuration flag:
     * BCU1: value memory segment: 0=ram, 1=eeprom
     * BCU2: memory segment selector (segment 0 or 1)
     */
    COMCONF_VALUE_TYPE = 0x20,

    /** Com object configuration flag: write enabled */
    COMCONF_WRITE = 0x10,

    /** Com object configuration flag: read enabled */
    COMCONF_READ = 0x08,

    /** Com object configuration flag: communication enabled */
    COMCONF_COMM = 0x04,

    /** Com object configuration combined flags: transmit + communication enabled */
    COMCONF_TRANS_COMM = 0x44,

    /** Com object configuration combined flags: read + communication enabled */
    COMCONF_READ_COMM = 0x0c,

    /** Com object configuration combined flags: write + communication enabled */
    COMCONF_WRITE_COMM = 0x14,

    /** Com object configuration flag: transmission priority mask */
    COMCONF_PRIO_MASK = 0x03,

    /** Com object configuration flag: low transmission priority */
    COMCONF_PRIO_LOW = 0x03,

    /** Com object configuration flag: high transmission priority */
    COMCONF_PRIO_HIGH = 0x02,

    /** Com object configuration flag: alarm transmission priority */
    COMCONF_PRIO_ALARM = 0x01,

    /** Com object configuration flag: system transmission priority */
    COMCONF_PRIO_SYSTEM = 0x00,
};

/**
 * Depending on the BCU type define the size of the pointer to the RAM
 * location of the value for a com object
 */
#if BCU_TYPE != BCU1_TYPE
typedef word DataPtrType;
#else
typedef byte DataPtrType;
#endif
/**
 * A communication object configuration.
 */
struct ComConfig
{
    /** Data pointer, low byte. Depending on the COMCONF_VALUE_TYPE flag in the
     * config byte, this pointer points to userRam or userEeprom.
     */
    DataPtrType dataPtr;

    /** Configuration flags. See enum ComConfigFlag. */
    byte config;

    /** Type of the communication object (bits 0..5). See enum ComType. */
    byte type;
};

/**
 * The flags for the communication object status.
 * 4 bits are used:
 * bit 0+1 are the transmission status: COMFLAG_OK, COMFLAG_ERROR, COMFLAG_TRANS, COMFLAG_TRANSREQ
 * bit 2 is the data request flag - set it to request an update
 * bit 3 is the update flag - it's set when the communication object was updated by the lib
 */
enum ComFlag
{
    /** Communication transmission status flag: idle/ok */
    COMFLAG_OK = 0x0,

    /** Communication transmission status flag: idle/error */
    COMFLAG_ERROR = 0x1,

    /** Communication transmission status flag: transmitting */
    COMFLAG_TRANS = 0x2,

    /**
     * Communication transmission status flag: transmission request
     *
     * This status is set by the application to request a transmission of the communication object.
     */
    COMFLAG_TRANSREQ = 0x3,

    /** Communication transmission status mask */
    COMFLAG_TRANS_MASK = 0x3,

    /**
     * Communication status flag: data request: 0=idle/response, 1=data request
     */
    COMFLAG_DATAREQ = 0x4,

    /**
     * Communication status flag: update: 0=not updated, 1=updated.
     *
     * This flag is set by the lib to inform the application that the value of a communication
     * object was updated by a telegram.
     */
    COMFLAG_UPDATE = 0x8,

    /** Mask for all communication flags */
    COMFLAG_MASK = 0x15
};

#endif /*sblib_eib_types_h*/
