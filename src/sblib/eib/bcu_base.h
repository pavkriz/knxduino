/*
 *  bcubase.h - BCU specific stuff.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_BcuBase_h
#define sblib_BcuBase_h

#include "../types.h"
#include "bus.h"
#include "bcu_type.h"
#include "properties.h"
#include "user_memory.h"
#include "../utils.h"


// Rename the method begin_BCU() of the class BCU to indicate the BCU type. If you get a
// link error then the library's BCU_TYPE is different from the application's BCU_TYPE.
#define begin_BCU  CPP_CONCAT_EXPAND(begin_,BCU_NAME)

class BcuBase;

/**
 * The EIB bus coupling unit.
 */
extern BcuBase& bcu;


/**
 * Class for controlling all BCU related things.
 *
 * In order to use the EIB bus, you need to call bcu.begin() once in your application's
 * setup() function.
 */
class BcuBase
{
public:
	BcuBase();

    /**
     * Begin using the EIB bus coupling unit, and set the  manufacturer-ID, device type,
     * and program version.
     *
     * @param manufacturer - the manufacturer ID (16 bit)
     * @param deviceType - the device type (16 bit)
     * @param version - the version of the application program (8 bit)
     */
    void begin(int manufacturer, int deviceType, int version);

    // /**
    //  * Set RxPin of board, must be called before begin method
    //  * @param rxPin pin definition
    //  */
    // void setRxPin(int rxPin) {
    //     bus.rxPin=rxPin;
    // }
    // /**
    //  * Set TxPin of board, must be called before begin method
    //  * @param txPin pin definition
    //  */
    // void setTxPin(int txPin) {
    //     bus.txPin=txPin;
    // }
    // /**
    //  * Set timer class, must be called before begin method
    //  * @param timer
    //  */
    // void setTimer(Timer& timer) {
    //     bus.timer=timer;
    // }
    // /**
    //  * Set capture channel of processor, must be called before begin method
    //  * @param capture channel definition of processor
    //  */
    // void setCaptureChannel(TimerCapture captureChannel) {
    //     bus.captureChannel=captureChannel;
    // }
    /**
     * Set ProgPin of board, must be called before begin method
     * @param progPin Pin definition
     */
    void setProgPin(int prgPin) {
        progPin=prgPin;
    }
    /**
     * Set ProgPin output inverted, must be called before begin method
     * @param progPin output inverted
     */
    void setProgPinInverted(int prgPinInv) {
        progPinInv=prgPinInv;
    }
    /**
     * End using the EIB bus coupling unit.
     */
    virtual void end();

    /**
     * Set our own physical address. Normally the physical address is set by ETS when
     * programming the device.
     *
     * @param addr - the physical address
     */
    void setOwnAddress(int addr);

    /**
     * Get our own physical address.
     */
    int ownAddress() const;

    /**
     * Test if the programming mode is active. This is also indicated
     * by the programming mode LED.
     *
     * @return True if the programming mode is active, false if not.
     */
    bool programmingMode() const;

    /**
     * Test if the user application is active. The application is active if the
     * application layer is active in userRam.status, the programming mode is not
     * active, and the run error in userEeprom.runError is 0xff (no error).
     *
     * @return True if the user application is active, false if not.
     */
    bool applicationRunning() const;

    /**
     * Test if a direct data connection is open.
     *
     * @return True if a connection is open, false if not.
     */
    bool directConnection() const;

    /**
     * Process the received telegram from bus.telegram.
     * Called by main()
     */
    virtual void processTelegram();

    /**
     * Get the mask version.
     * Usually 0x0012 for BCU1, 0x0020 for BCU2.
     */
    int maskVersion() const;

    /**
     * The BCU's main processing loop. This is like the application's loop() function,
     * and is called automatically by main() when the BCU is activated with bcu.begin().
     */
    virtual void loop();

    /**
     * A buffer for sending telegrams. This buffer is considered library private
     * and should rather not be used by the application program.
     */
    byte sendTelegram[Bus::TELEGRAM_SIZE];

    /**
     * The pin where the programming LED + button are connected. The default pin
     * is PIO1_5. This variable may be changed in setup(), if required. If set
     * to 0, the programming LED + button are not handled by the library.
     */
    int progPin;

    /**
     * Programming LED output inverted: If set to 1 the programming LED output is
     * being inverted
     */
    int progPinInv;

protected:
    // The method begin_BCU() is renamed during compilation to indicate the BCU type.
    // If you get a link error then the library's BCU_TYPE is different from your application's BCU_TYPE.
    void begin_BCU(int manufacturer, int deviceType, int version);
    /*
     * Special initialization for the BCU
     */
    virtual void _begin();

    //Debouncer progButtonDebouncer; //!< The debouncer for the programming mode button.
    bool enabled;                  //!< The BCU is enabled. Set by bcu.begin().
    int  connectedAddr;            //!< Remote address of the connected partner.
    byte sendCtrlTelegram[8];      //!< A short buffer for connection control telegrams.
    int  connectedSeqNo;           //!< Sequence number for connected data telegrams.
    unsigned int connectedTime;    //!< System time of the last connected telegram.
    bool incConnectedSeqNo;        //!< True if the sequence number shall be incremented on ACK.
    int lastAckSeqNo;              //!< Last acknowledged sequence number
};


//
//  Inline functions
//

inline void BcuBase::begin(int manufacturer, int deviceType, int version)
{
    begin_BCU(manufacturer, deviceType, version);
}

inline bool BcuBase::programmingMode() const
{
    return (userRam.status & BCU_STATUS_PROG) == BCU_STATUS_PROG;
}

inline int BcuBase::ownAddress() const
{
    return bus.ownAddr;
}

inline bool BcuBase::applicationRunning() const
{
    if (!enabled)
        return false;

#if BCU_TYPE == BCU1_TYPE
    return (userRam.status & (BCU_STATUS_PROG|BCU_STATUS_AL)) == BCU_STATUS_AL &&
        userRam.runState == 1 && userEeprom.runError == 0xff; // ETS sets the run error to 0 when programming
#else
    return userRam.runState == 1 &&
        userEeprom.loadState[OT_ADDR_TABLE] == LS_LOADED &&  // Address table object
        userEeprom.loadState[OT_ASSOC_TABLE] == LS_LOADED && // Association table object &
        userEeprom.loadState[OT_APPLICATION] == LS_LOADED;   // Application object. All three in State "Loaded"
#endif
}

inline int BcuBase::maskVersion() const
{
    return MASK_VERSION;
}

inline bool BcuBase::directConnection() const
{
    return connectedAddr != 0;
}

#ifndef INSIDE_BCU_CPP
#   undef begin_BCU
#endif

#endif /*sblib_bcu_h*/
