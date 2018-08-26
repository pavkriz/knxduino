/*
 *  property_types.h - BCU 2 property types of EIB objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_property_types_h
#define sblib_property_types_h

#include "../types.h"
#include "../utils.h"
#include "bcu_type.h"

// See BCU2 help:
// System Architecture > Interface Objects > User Interface Objects > Attributes of Properties


/**
 * Data type of a property.
 */
enum PropertyDataType
{
    PDT_CONTROL = 0,              //!< length: 1 read, 10 write
    PDT_CHAR = 1,                 //!< length: 1
    PDT_UNSIGNED_CHAR = 2,        //!< length: 1
    PDT_INT = 3,                  //!< length: 2
    PDT_UNSIGNED_INT = 4,         //!< length: 2
    PDT_EIB_FLOAT = 5,            //!< length: 2
    PDT_DATE = 6,                 //!< length: 3
    PDT_TIME = 7,                 //!< length: 3
    PDT_LONG = 8,                 //!< length: 4
    PDT_UNSIGNED_LONG = 9,        //!< length: 4
    PDT_FLOAT = 0x0a,             //!< length: 4
    PDT_DOUBLE = 0x0b,            //!< length: 8
    PDT_CHAR_BLOCK = 0x0c,        //!< length: 10
    PDT_POLL_GROUP_SETTING = 0x0d,//!< length: 3
    PDT_SHORT_CHAR_BLOCK = 0x0e,  //!< length: 5
    //---
    PDT_GENERIC_01 = 0x11,        //!< length: 1
    PDT_GENERIC_02,          //!< length: 2
    PDT_GENERIC_03,          //!< length: 3
    PDT_GENERIC_04,          //!< length: 4
    PDT_GENERIC_05,          //!< length: 5
    PDT_GENERIC_06,          //!< length: 6
    PDT_GENERIC_07,          //!< length: 7
    PDT_GENERIC_08,          //!< length: 8
    PDT_GENERIC_09,          //!< length: 9
    PDT_GENERIC_10,          //!< length: 10
};


/**
 * Definition of a property.
 */
struct PropertyDef
{
    /**
     * The ID of the property.
     */
    byte id;

    /**
     * The control byte contains bits that define the characteristics of a property.
     *
     * Bit 0..4: type, see PropertyDataType
     * Bit 5: the property value is a pointer
     * Bit 6: the property is an array
     * Bit 7: the property is writable
     */
    byte control;

    /**
     * Address of the property value, or the value itself if bit 5 of control is unset.
     * If it is a pointer, it points into BCU2 address space: userEeprom or userRam.
     * Bit 15 is special if valPtr contains a pointer: then the pointer points to a function.
     *
     * See valuePointer() below.
     */
    word valAddr;

    /**
     * Get the value pointer.
     *
     * @return The pointer to the property value.
     */
    byte* valuePointer() const;

    /**
     * Test if the valuePointer() points to the userEeprom.
     *
     * @return True if it is an EEPROM pointer, false if not.
     */
    bool isEepromPointer() const;

    /**
     * Get the data type of the property.
     *
     * @return The property type, see PropertyDataType
     */
    PropertyDataType type() const;

    /**
     * Get the size of the property in bytes.
     *
     * @return The size of the property.
     */
    int size() const;
};


/**
 * Property ID.
 */
// See KNX 3/7/3 Standardized Identifier Tables, p. 17 for property data types
enum PropertyID
{
    /** Generic object property: interface object type. */
    PID_OBJECT_TYPE = 1,

    /** Application object property: load state control. */
    PID_LOAD_STATE_CONTROL = 5,

    /** Application object property: run state control. */
    PID_RUN_STATE_CONTROL = 6,

    /** Application object property: table address. */
    PID_TABLE_REFERENCE = 7,

    /** Device object property: service control. */
    PID_SERVICE_CONTROL = 8,

    /** Device object property: firmware revision. */
    PID_FIRMWARE_REVISION = 9,

    /** Device object property: serial number. */
    PID_SERIAL_NUMBER = 11,

    /** Device object property: manufacturer ID. */
    PID_MANUFACTURER_ID = 12,

    /** Application object property: program version. */
    PID_PROG_VERSION = 13,

    /** Device object property: device control. */
    PID_DEVICE_CONTROL = 14,

    /** Device object property: order info. */
    PID_ORDER_INFO = 15,

    /** Device object property: PEI type. */
    PID_PEI_TYPE = 16,

    /** Device object property: port configuration. */
    PID_PORT_CONFIGURATION = 17,

    /** Device object property: hardware type. */
    PID_HARDWARE_TYPE = 78,

    PID_ABB_CUSTOM = 0xcc,
};


/**
 * Property control constants.
 */
enum PropertyControl
{
    PC_WRITABLE = 0x80,       //!< The property can be modified
    PC_ARRAY = 0x40,          //!< The property is an array (max. 255 bytes)
    PC_POINTER = 0x20,        //!< valPtr of the property definition is a pointer
    PC_ARRAY_POINTER = 0x60,  //!< Combination of PC_ARRAY|PC_POINTER
    PC_TYPE_MASK = 0x1f       //!< Bit mask for the property type
};


/**
 * Property pointer type.
 */
enum PropertyPointerType
{
    PPT_USER_RAM = 0,         //!< Pointer to user RAM
    PPT_USER_EEPROM = 0x4000, //!< Pointer to user EEPROM
    PPT_MASK = 0x7000,        //!< Bitmask for property pointer types
    PPT_OFFSET_MASK = 0x0fff  //!< Bitmask for property pointer offsets
};


/** Define a PropertyDef pointer to variable v in the userRam */
#define PD_USER_RAM_OFFSET(v) (OFFSET_OF(UserRam, v) + PPT_USER_RAM)

/** Define a PropertyDef pointer to variable v in the userEeprom */
#define PD_USER_EEPROM_OFFSET(v) (OFFSET_OF(UserEeprom, v) + PPT_USER_EEPROM)

/** Define a PropertyDef pointer to variable v in the internal constants table */
#define PD_CONSTANTS_OFFSET(v) (OFFSET_OF(ConstPropValues, v) + PPT_CONSTANTS)

/** Mark the end of a property definition table */
#define PROPERTY_DEF_TABLE_END  { 0, 0, 0 }


//
//  Inline functions
//

inline PropertyDataType PropertyDef::type() const
{
    return (PropertyDataType) (control & 31);
}

inline bool PropertyDef::isEepromPointer() const
{
    return (valAddr & PPT_USER_EEPROM) != 0;
}

#endif /*sblib_property_types_h*/
