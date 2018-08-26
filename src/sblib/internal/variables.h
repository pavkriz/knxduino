/*
 *  internal/variables.h - Library internal shared variables
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_internal_variables_h
#define sblib_internal_variables_h

#include "../eib/properties.h"
#include "../eib/user_memory.h"

// System time in milliseconds (from timer.cpp)
extern volatile unsigned int systemTime;

// user_memory.cpp
extern byte userRamData[USER_RAM_SIZE+USER_RAM_SHADOW_SIZE];

// user_memory.cpp
extern byte userEepromData[USER_EEPROM_SIZE];

// user_memory.cpp
extern byte userEepromDirty;

// Number of interface/property objects
#define NUM_PROP_OBJECTS 4


#if BCU_TYPE != BCU1_TYPE
// Table of interface/property objects (eib/property_defs.cpp)
extern const PropertyDef* const propertiesTab[NUM_PROP_OBJECTS];

#endif

#endif /*sblib_internal_variables_h*/
