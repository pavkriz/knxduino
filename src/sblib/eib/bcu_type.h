/*
 *  bcu_type.h - BCU type definitions.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_bcu_type_h
#define sblib_bcu_type_h


//
// Set the BCU_TYPE from compiler defines
//

#if defined(BCU2) || BCU_TYPE == 0x20 || BCU_TYPE == 20
//  BCU 2, mask version 2.0
#   undef BCU_TYPE
#   define BCU_TYPE 0x20
#   define BCU_NAME BCU2
#   define MASK_VERSION 0x20

    /** Offset between the object index and the payload data
     *  if the property commands are simulated via memory read/writes.
     *  Only required for BIM112
     */
#   define PROP_LOAD_OFFSET 0

#elif defined(BIM112) || defined(BIM112_71) || BCU_TYPE == 0x701
//  BIM 112, mask version 7.1
#   undef BCU_TYPE
#   define BCU_TYPE 0x701
#   define BCU_NAME BIM112
#   define MASK_VERSION 0x701

    /** Address for load control */
#   define LOAD_CONTROL_ADDR 0x104

    /** Offset between the object index and the payload data
     *  if the property commands are simulated via memory read/writes
     */
#   define PROP_LOAD_OFFSET 1

/** Address for load state */
#   define LOAD_STATE_ADDR 0xb6e9


#elif defined(BCU1) || defined(BCU1_12) || BCU_TYPE == 0x10 || BCU_TYPE == 10 || !defined(BCU_TYPE)
//  BCU 1, mask version 1.2
//  Also use BCU 1 if no BCU type is set
#   undef BCU_TYPE
#   define BCU_TYPE 0x10
#   define BCU_NAME BCU1
#   define MASK_VERSION 0x12

    /** Offset between the object index and the payload data
     *  if the property commands are simulated via memory read/writes.
     *  Only required for BIM112
     */
#   define PROP_LOAD_OFFSET 0

#else
//  Unknown BCU type
#   error "Unknown BCU type. Please add a compiler define: either BCU_TYPE with the correct value or one of the predefined BCU types - see sblib/eib/bcu_type.h for valid types"
#endif

// The BCU 1 type, for comparison
#define BCU1_TYPE 0x10

// The BIM-112 type, for comparison
#define BIM112_TYPE 0x701



#if BCU_TYPE == 0x10  /* BCU 1 */

    /** Start address of the user RAM when ETS talks with us. */
#   define USER_RAM_START_DEFAULT 0

    /** The size of the user RAM in bytes. */
#   define USER_RAM_SIZE 0x100
	/** How many bytes have to be allocated at the end of the RAM
		for shadowed values
	*/
#   define USER_RAM_SHADOW_SIZE 3

    /** Start address of the user EEPROM when ETS talks with us. */
#   define USER_EEPROM_START 0x100

    /** The size of the user EEPROM in bytes. */
#   define USER_EEPROM_SIZE 256

#elif BCU_TYPE == 0x20  /* BCU 2 */

    /** Start address of the user RAM when ETS talks with us. */
#   define USER_RAM_START_DEFAULT 0

    /** The size of the user RAM in bytes. */
#   define USER_RAM_SIZE 0x100
	/** How many bytes have to be allocated at the end of the RAM
		for shadowed values
	*/
#   define USER_RAM_SHADOW_SIZE 0

    /** Start address of the user EEPROM when ETS talks with us. */
#   define USER_EEPROM_START 0x100

    /** The size of the user EEPROM in bytes. */
#   define USER_EEPROM_SIZE 1024

#elif BCU_TYPE == 0x701  /* BIM 112, v7.1 */

    /** Start address of the user RAM when ETS talks with us. */
#   define USER_RAM_START_DEFAULT 0x5FC

#		ifndef EXTRA_USER_RAM_SIZE
#			define EXTRA_USER_RAM_SIZE 0
#		endif
    /** The size of the user RAM in bytes. */
#   define USER_RAM_SIZE (0x304 + EXTRA_USER_RAM_SIZE)

    /** How many bytes have to be allocated at the end of the RAM for shadowed values */
#   define USER_RAM_SHADOW_SIZE 3

    /** Start address of the user EEPROM when ETS talks with us. */
#   define USER_EEPROM_START 0x3f00

    /** The size of the user EEPROM in bytes. */
#   define USER_EEPROM_SIZE       3072
#   define USER_EEPROM_FLASH_SIZE 4096

#else
    // BCU_TYPE contains an invalid value and no other BCU type define is set
#   error "Unsupported BCU type"
#endif

#ifndef USER_EEPROM_FLASH_SIZE
#define USER_EEPROM_FLASH_SIZE USER_EEPROM_SIZE
#endif

/** End address of the user EEPROM +1, when ETS talks with us. */
#define USER_EEPROM_END (USER_EEPROM_START + USER_EEPROM_SIZE)


#endif /*sblib_bcu_type_h*/
