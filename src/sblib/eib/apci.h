/*
 *  apci.h - Telegram application types.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_apci_h
#define sblib_apci_h

enum
{
    // Application commands (see KNX 3/3/7 p.8 Application Layer control field)

    APCI_GROUP_MASK = 0x3c0,

    APCI_GROUP_VALUE_READ_PDU = 0x000,
    APCI_GROUP_VALUE_RESPONSE_PDU = 0x040,
    APCI_GROUP_VALUE_WRITE_PDU = 0x080,
    APCI_INDIVIDUAL_ADDRESS_WRITE_PDU = 0x0c0,
    APCI_INDIVIDUAL_ADDRESS_READ_PDU = 0x100,
    APCI_INDIVIDUAL_ADDRESS_RESPONSE_PDU = 0x140,

    APCI_ADC_READ_PDU = 0x180,
    APCI_ADC_RESPONSE_PDU = 0x1C0,

    APCI_MEMORY_READ_PDU = 0x200,
    APCI_MEMORY_RESPONSE_PDU = 0x240,
    APCI_MEMORY_WRITE_PDU = 0x280,

    APCI_DEVICEDESCRIPTOR_READ_PDU = 0x300,
    APCI_DEVICEDESCRIPTOR_RESPONSE_PDU = 0x340,

    APCI_RESTART_PDU = 0x380,
	APCI_RESTART_TYPE1_PDU = 0x381,

    APCI_AUTHORIZE_REQUEST_PDU = 0x3d1,
    APCI_AUTHORIZE_RESPONSE_PDU = 0x3d2,

    APCI_PROPERTY_VALUE_READ_PDU = 0x3d5,
    APCI_PROPERTY_VALUE_RESPONSE_PDU = 0x3d6,
    APCI_PROPERTY_VALUE_WRITE_PDU = 0x3d7,

    APCI_PROPERTY_DESCRIPTION_READ_PDU = 0x3d8,
    APCI_PROPERTY_DESCRIPTION_RESPONSE_PDU = 0x3d9,

    // Transport commands

    T_CONNECT_PDU = 0x80,
    T_DISCONNECT_PDU = 0x81,
    T_ACK_PDU = 0xc2,
    T_NACK_PDU = 0xc3,

    // TPCI (TPDU) commands

    T_GROUP_PDU = 0x00
};


#endif /*sblib_apci_h*/
