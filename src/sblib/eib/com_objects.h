/*
 *  com_objects.h - EIB Communication objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_com_objects_h
#define sblib_com_objects_h

//#include <sys/param.h>
#include "bcu.h"
#include "types.h"
#include "datapoint_types.h"

extern int le_ptr;

inline void objectEndian(int val)
{
	le_ptr=val;
}

/**
 * Get the numeric value from a communication object. Can be used for
 * communication objects of type 1 bit up to type 4 bytes.
 *
 * @param objno - the ID of the communication object.
 * @return The value of the com-object.
 */
unsigned int objectRead(int objno);

/**
 * Get the float value from a communication object. Can be used for
 * communication objects of type 2 byte float (EIS5 / DPT9). The value is in
 * 1/100 - a DPT9 value of 21.01 is returned as 2101.
 *
 * @param objno - the ID of the communication object.
 * @return The value of the com-object in 1/100. INVALID_DPT_FLOAT is returned
 *         for the DPT9 "invalid data" value.
 */
float objectReadFloat(int objno);

/**
 * Request the read of a communication object. Calling this function triggers the
 * sending of a read-group-value telegram, to read the value of the communication
 * object from the bus.
 *
 * When the answer is received, the communication object's value will be updated.
 * You can cycle through all updated communication objects with nextUpdatedObject().
 *
 * @param objno - the ID of the communication object to mark for reading.
 *
 * @see objectWritten(int)
 */
void requestObjectRead(int objno);

/**
 * Get a pointer to the value bytes of the communication object. Can be used for
 * any communication object. The minimum that is used for a communication object
 * is 1 byte. Use objectSize(objno) to get the size of the communication object's
 * value.
 *
 * @param objno - the ID of the communication object.
 * @return The value of the com-object.
 */
byte* objectValuePtr(int objno);

/**
 * Get the size of the communication object's value in bytes.
 *
 * @param objno - the ID of the communication object.
 * @return The size in bytes.
 */
int objectSize(int objno);

/**
 * Set the value of a communication object without triggering the
 * sending of a write-group-value telegram.
 *
 * @param objno - the ID of the communication object.
 * @param value - the new value of the communication object.
 */
void objectSetValue(int objno, unsigned int value);

/**
 * Set the value of a communication object. Calling this function triggers the
 * sending of a write-group-value telegram.
 *
 * @param objno - the ID of the communication object.
 * @param value - the new value of the communication object.
 */
void objectWrite(int objno, unsigned int value);

/**
 * Set the value of a communication object. Calling this function triggers the
 * sending of a write-group-value telegram. The number of bytes that are copied
 * depends on the size of the target communication object's field.
 *
 * @param objno - the ID of the communication object.
 * @param value - the new value of the communication object.
 */
void objectWrite(int objno, byte* value);

/**
 * Set the value of a communication object. Calling this function triggers the
 * sending of a write-group-value telegram.
 *
 * The communication object is a 2 byte float (EIS5 / DPT9) object. The value is
 * in 1/100, so a value of 2101 would set a DPT9 float value of 21.01. The valid
 * range of the values is -671088.64 to 670760.96.
 *
 * @param objno - the ID of the communication object.
 * @param value - the new value of the communication object in 1/100.
 *                Use INVALID_DPT_FLOAT for the DPT9 "invalid data" value.
 */
void objectWriteFloat(int objno, int value);

/**
 * Mark a communication object as written. Use this function if you directly change
 * the value of a communication object without using objectWrite(). Calling this
 * function triggers the sending of a write-group-value telegram.
 *
 * @param objno - the ID of the communication object.
 *
 * @see requestObjectRead(int)
 */
void objectWritten(int objno);

/**
 * Set the value of a communication object and mark the communication object
 * as updated. This does not trigger a write-group-value telegram.
 *
 * @param objno - the ID of the communication object.
 * @param value - the new value of the communication object.
 */
void objectUpdate(int objno, unsigned int value);

/**
 * Set the value of a communication object and mark the communication object
 * as updated. This does not trigger a write-group-value telegram.
 *
 * @param objno - the ID of the communication object.
 * @param value - the new value of the communication object.
 */
void objectUpdate(int objno, byte* value);

/**
 * Set the value of a communication object and mark the communication object
 * as updated. This does not trigger a write-group-value telegram.
 *
 * The communication object is a 2 byte float (EIS5 / DPT9) object. The value
 * is in 1/100, so a value of 2101 would set a DPT9 float value of 21.01.
 * The possible range of the values is -671088.64 to 670760.96.
 *
 * @param objno - the ID of the communication object.
 * @param value - the new value of the communication object in 1/100.
 *                Use INVALID_DPT_FLOAT for the DPT9 "invalid data" value.
 */
void objectUpdateFloat(int objno, int value);

/**
 * Update the value of the communication object without changing the flags
 *
 * @param objno - the ID of the communication object.
 * @param value - the new value of the communication object.
 */
void setObjectValue(int objno, unsigned int value);

/**
 * Get the ID of the next communication object that was updated
 * over the bus by a write-value-request telegram.
 *
 * @return The ID of the next updated com-object, -1 if none.
 */
int nextUpdatedObject();

/**
 * Get the type of a communication object.
 *
 * @param objno - the ID of the communication object.
 * @return The type of the communication object.
 */
ComType objectType(int objno);

/**
 * Returns the config of the specified communication object.
 *
 * @param objno The ID of the communication object
 * @return The communication object's configuration.
 *
 * Remark:
 * The first byte of the com table is the number of entries, followed
 * by the address of the RAM area used by the com objects. After that,
 * the configuration for each com object follows.
 * The sizes of the RAM pointer and the com object varies between different
 * BCU types. Therefore the sizeof operator is used instead of hard-coded
 * values.
 */
const ComConfig& objectConfig(int objno);

/**
 * Process a multicast group telegram.
 *
 * This function is called by bcu.processTelegram(). It is usually not required to call
 * this function from within a user program.
 *
 * @param addr - the destination group address.
 * @apci - Kind of telegram to be processed
 *         APCI_GROUP_VALUE_WRITE_PDU
 *         APCI_GROUP_VALUE_RESPONSE_PDU
 *         APCI_GROUP_VALUE_READ_PDU
 * @tel - pointer to the telegram to read from
 *         (only used if APCI_GROUP_VALUE_WRITE_PDU or APCI_GROUP_VALUE_RESPONSE_PDU)
 */
void processGroupTelegram(int addr, int apci, byte* tel);

/**
 * Get the communication object configuration table ("COMMS" table). This is the table
 * with the flags that are configured by ETS (not the RAM status flags).
 *
 * @return The com-objects configuration table.
 *
 * @brief The first byte of the table contains the number of entries. The second
 * byte contains the address of the object status flags in userRam. The rest of
 * the table consists of the ComConfig objects - 3 bytes per communication
 * object.
 */
byte* objectConfigTable();

/**
 * Get the communication object status flags table. This is the table with the
 * status flags that are stored in RAM and get changed during normal operation.
 *
 * @return The com-objects status flags table.
 *
 * @brief The whole table consists of the status flags - 4 bits per communication
 * object.
 */
byte* objectFlagsTable();


//
//  Inline functions
//

inline ComType objectType(int objno)
{
    return (ComType) objectConfig(objno).type;
}

inline const ComConfig& objectConfig(int objno)
{
    return *(const ComConfig*) (objectConfigTable() + 1 + sizeof(DataPtrType) + objno * sizeof(ComConfig) );
}

inline void requestObjectRead(int objno)
{
    extern void setObjectFlags(int objno, int flags);
    setObjectFlags(objno, COMFLAG_TRANSREQ | COMFLAG_DATAREQ);
}

inline void objectWritten(int objno)
{
    extern void addObjectFlags(int objno, int flags);
    addObjectFlags(objno, COMFLAG_TRANSREQ);
}

inline void objectSetValue(int objno, unsigned int value)
{
    extern void _objectWrite(int objno, unsigned int val, int flags);
    _objectWrite(objno, value, 0);
}

inline void objectWrite(int objno, unsigned int value)
{
    extern void _objectWrite(int objno, unsigned int val, int flags);
    _objectWrite(objno, value, COMFLAG_TRANSREQ);
}

inline void objectWrite(int objno, byte* value)
{
    extern void _objectWriteBytes(int objno, byte* val, int flags);
    _objectWriteBytes(objno, value, COMFLAG_TRANSREQ);
}

inline void objectWriteFloat(int objno, int value)
{
    extern void _objectWrite(int objno, unsigned int val, int flags);
    _objectWrite(objno, dptToFloat(value), COMFLAG_TRANSREQ);
}

inline void objectUpdate(int objno, unsigned int value)
{
    extern void _objectWrite(int objno, unsigned int val, int flags);
    _objectWrite(objno, value, COMFLAG_UPDATE);
}

inline void objectUpdate(int objno, byte* value)
{
    extern void _objectWriteBytes(int objno, byte* val, int flags);
    _objectWriteBytes(objno, value, COMFLAG_UPDATE);
}

inline void objectUpdateFloat(int objno, int value)
{
    extern void _objectWrite(int objno, unsigned int val, int flags);
    _objectWrite(objno, dptToFloat(value), COMFLAG_UPDATE);
}

inline float objectReadFloat(int objno)
{
    return dptFromFloat(objectRead(objno));
}

#endif /*sblib_com_objects_h*/
