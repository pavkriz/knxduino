/*
 *  internal/functions.h - Library internal shared functions
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_internal_functions_h
#define sblib_internal_functions_h

#include "../eib/property_types.h"

/*
 *  These functions are private library functions. Do not call from
 *  outside the library. They may be changed without warning.
 */


/*
 * Read userEeprom from Flash. (user_memory.cpp)
 */
void readUserEeprom();

// Write userEeprom to Flash. (user_memory.cpp)
void writeUserEeprom();

/*
 * Send the next communication object that is flagged to be sent.
 * Returns true if a group telegram has been sent.
 */
bool sendNextGroupTelegram();

/*
 * Process a property-value read telegram. (properties.cpp)
 *
 * @param objectIdx - the object index
 * @param propertyId - the property ID
 * @param count - the number of elements to read
 * @param start - the index of the first element to read
 *
 * @return True if the request was processed, false if an error occured.
 */
bool propertyValueReadTelegram(int objectIdx, PropertyID propertyId, int count, int start);

/*
 * Process a property-value write telegram. (properties.cpp)
 * The data to write is stored in bus.telegram[12..]
 *
 * @param objectIdx - the object index
 * @param propertyId - the property ID
 * @param count - the number of elements to write
 * @param start - the index of the first element to write
 *
 * @return True if the request was processed, false if an error occured.
 */
bool propertyValueWriteTelegram(int objectIdx, PropertyID propertyId, int count, int start);

/*
 * Process a property-description read telegram.
 * (properties.cpp)
 *
 * @param objectIdx - the object index
 * @param propertyId - the property ID
 * @param propertyIdx - the index of the property, if it is an array
 *
 * @return True if the request was processed, false if an error occured.
 */
bool propertyDescReadTelegram(int objectIdx, PropertyID propertyId, int propertyIdx);


#endif /*sblib_internal_functions_h*/
