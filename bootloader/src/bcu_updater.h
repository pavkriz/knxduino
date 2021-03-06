/*
 * BCUUpdate.h
 *
 *  Created on: 15.07.2015
 *      Author: glueck
 */

#ifndef BCU_UPDATE_H_
#define BCU_UPDATE_H_

#include <eib.h>
#include <eib/bcu_base.h>
#include <eib/apci.h>
#include <internal/variables.h>

class BcuUpdate: public BcuBase
{
public:
    virtual void processTelegram();
    bool progPinStatus();
protected:
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
     * Process a unicast connection control telegram with our physical address as
     * destination address. The telegram is stored in sbRecvTelegram[].
     *
     * When this function is called, the sender address is != 0 (not a broadcast).
     *
     * @param tpci - the transport control field
     */
    void processConControlTelegram(int tpci);

    /**
     * Send a connection control telegram.
     *
     * @param cmd - the transport command, see SB_T_xx defines
     * @param senderSeqNo - the sequence number of the sender, 0 if not required
     */
    void sendConControlTelegram(int cmd, int senderSeqNo);

    void dumpTelegram();

    void loop() override;
};

inline bool BcuUpdate::progPinStatus()
{
    //return progButtonDebouncer.value();
	//return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);
	return false;
}
#endif /* BCU_UPDATE_H_ */
