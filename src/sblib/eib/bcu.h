/*
 *  bcu.h - BCU specific stuff.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_bcu_h
#define sblib_bcu_h

#include "bcu_base.h"
#include "../types.h"
#include "bus.h"
#include "bcu_type.h"
#include "properties.h"
#include "user_memory.h"
#include "../utils.h"
#include "../mem_mapper.h"
#include "../usr_callback.h"


// Rename the method begin_BCU() of the class BCU to indicate the BCU type. If you get a
// link error then the library's BCU_TYPE is different from the application's BCU_TYPE.
#define begin_BCU  CPP_CONCAT_EXPAND(begin_,BCU_NAME)

/**
 * Class for controlling all BCU related things.
 *
 * In order to use the EIB bus, you need to call bcu.begin() once in your application's
 * setup() function.
 */
class BCU : public BcuBase
{
public:
    virtual void processTelegram();

    /**
     * The BCU's main processing loop. This is like the application's loop() function,
     * and is called automatically by main() when the BCU is activated with bcu.begin().
     */
    virtual void loop();

    int connectedTo();

    /**
     * Allow an user provided memory mapper to store parameter data via memory write / read
     * @param mapper - a pointer to an instance of a MemMapper object
     */
    void setMemMapper(MemMapper *mapper);

    /**
     * Set a callback class to notify the user program of some events
     */
    void setUsrCallback(UsrCallback *callback);

    /**
     * End using the EIB bus coupling unit.
     */
    virtual void end();

    /**
     * Enable/Disable sending of group write or group response telegrams.
     * Useful if the device wants to implement transmission delays
     * after bus voltage recovery.
     * Transmission is enabled by default.
     */
    void enableGroupTelSend(bool enable);

    /**
     * Set a limit for group telegram tramsissions per second.
     * If the parameter is not zero, there is a minimum delay
     * of 1/limit (in seconds) between subsequent group telegram
     * transmissions.
     *
     * @param limit - the maximum number of telegrams per second.
     */
    void setGroupTelRateLimit(unsigned int limit);

protected:
    /*
     * Special initialization for the BCU
     */
    virtual void _begin();
    /**
     * Process a unicast connection control telegram with our physical address as
     * destination address. The telegram is stored in sbRecvTelegram[].
     *
     * When this function is called, the sender address is != 0 (not a broadcast).
     *
     * @param tpci - the transport control field
     */
    void processConControlTelegram(int tpci);

    /**
     * Process a unicast telegram with our physical address as destination address.
     * The telegram is stored in sbRecvTelegram[].
     *
     * When this function is called, the sender address is != 0 (not a broadcast).
     *
     * @param apci - the application control field
     */
    void processDirectTelegram(int apci);

    /**
     * Send a connection control telegram.
     *
     * @param cmd - the transport command, see SB_T_xx defines
     * @param senderSeqNo - the sequence number of the sender, 0 if not required
     */
    void sendConControlTelegram(int cmd, int senderSeqNo);

    /**
     * Process a device-descriptor-read request.
     *
     * @param id - the device-descriptor type ID
     *
     * @return True on success, false on failure
     */
    bool processDeviceDescriptorReadTelegram(int id);

    // The method begin_BCU() is renamed during compilation to indicate the BCU type.
    // If you get a link error then the library's BCU_TYPE is different from your application's BCU_TYPE.
    void begin_BCU(int manufacturer, int deviceType, int version);

private:
    MemMapper *memMapper;
    UsrCallback *usrCallback;
    bool sendGrpTelEnabled;        //!< Sending of group telegrams is enabled. Usually set, but can be disabled.
    unsigned int groupTelWaitMillis;
    unsigned int groupTelSent;
};


//
//  Inline functions
//

inline int BCU::connectedTo()
{
    return connectedAddr;
}

inline void BCU::setMemMapper(MemMapper *mapper)
{
    memMapper = mapper;
}

inline void BCU::setUsrCallback(UsrCallback *callback)
{
    usrCallback = callback;
}

inline void BCU::enableGroupTelSend(bool enable)
{
    sendGrpTelEnabled = enable;
}

inline void BCU::setGroupTelRateLimit(unsigned int limit)
{
 if ((limit > 0) && (limit <= 1000))
     groupTelWaitMillis = 1000/limit;
 else
     groupTelWaitMillis = 0;
}

#ifndef INSIDE_BCU_CPP
#   undef begin_BCU
#endif

#endif /*sblib_bcu_h*/
