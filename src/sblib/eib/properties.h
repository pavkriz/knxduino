/*
 *  properties.h - BCU 2 properties of EIB objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_properties_h
#define sblib_properties_h

#include "../types.h"
#include "property_types.h"

#if BCU_TYPE != BCU1_TYPE

/*
 * Load / configure a property. Called when a "load control" property-write telegram
 * is received.
 *
 * @param objectIdx - the ID of the interface object.
 * @param data - the data bytes
 * @param len - the length of the data.
 * @return The load state.
 */
int loadProperty(int objectIdx, const byte* data, int len);

/**
 * Find a property definition in a properties table.
 *
 * @param id - the ID of the property to find.
 * @param table - the properties table. The last table element must be PROPERTY_DEF_TABLE_END
 *
 * @return Pointer to the property definition, 0 if not found.
 */
const PropertyDef* findProperty(PropertyID id, const PropertyDef* table);

/**
 * Interface object type ID
 */
enum ObjectType
{
    /** Device object. */
    OT_DEVICE = 0,

    /** Address table object. */
    OT_ADDR_TABLE = 1,

    /** Association table object. */
    OT_ASSOC_TABLE = 2,

    /** Application program object. */
    OT_APPLICATION = 3
};

/**
 * Load states
 */
enum LoadState
{
    // No data is loaded
    LS_UNLOADED = 0,

    // Valid data is loaded
    LS_LOADED = 1,

    // Load process is active
    LS_LOADING = 2,

    // Error in data detected or error during load process
    LS_ERROR = 3,

    // Optional state: Unload process is active
    LS_UNLOADING = 4,

    // Optional state: Intermediate state between Loading and Loaded
    LS_LOADCOMPLETING = 5
};

#endif /*BCU_TYPE != BCU1_TYPE*/

#endif /*sblib_properties_h*/
