/*
 * BCUUpdate.cpp
 *
 *  Created on: 15.07.2015
 *      Author: glueck
 */

#include "bcu_updater.h"

#ifdef DUMP_TELEGRAMS
///#define d(x) {serial.println(x);}
#include "serial.h"
#define d(...) {UART2_printf(__VA_ARGS__);}
#else
#define d(...)
#endif

void BcuUpdate::dumpTelegram() {
	#ifdef DUMP_TELEGRAMS
		d("TLG: ");
		for (int i = 0; i <= bus.telegramLen; ++i) {
			d("%02X ", bus.telegram[i]);
		}
		d("\n");
	#endif
}

void BcuUpdate::processTelegram()
{
    unsigned short destAddr = (bus.telegram[3] << 8) | bus.telegram[4];
    unsigned char tpci = bus.telegram[6] & 0xc3; // Transport control field (see KNX 3/3/4 p.6 TPDU)
    unsigned short apci = ((bus.telegram[6] & 3) << 8) | bus.telegram[7];

    if ((bus.telegram[5] & 0x80) == 0) // a physical destination address
    {
        if (destAddr == bus.ownAddress()) // it's our physical address
        {
            if (tpci & 0x80)  // A connection control command
            {
                d("ProcCT\n");
                processConControlTelegram(bus.telegram[6]);
            }
            else
            {
                d("ProcDT\n");
                processDirectTelegram(apci);
            }
        }
    }
    // At the end: discard the received telegram
    bus.discardReceivedTelegram();
}

extern unsigned char handleMemoryRequests(int apciCmd, bool * sendTel,
        unsigned char * data);

void BcuUpdate::processDirectTelegram(int apci)
{
	dumpTelegram();

    const int senderAddr = (bus.telegram[1] << 8) | bus.telegram[2];
    const int senderSeqNo = bus.telegram[6] & 0x3c;
    unsigned char sendAck = 0;
    bool sendTel = false;

    if (connectedAddr != senderAddr) // ensure that the sender is correct
        return;

    connectedTime = HAL_GetTick();
    sendTelegram[6] = 0;

    int apciCmd = apci & APCI_GROUP_MASK;
    if ((apciCmd == APCI_MEMORY_READ_PDU) | (apciCmd == APCI_MEMORY_WRITE_PDU))
    {
        d("R/W DATA\n");
        sendAck = handleMemoryRequests(apciCmd, &sendTel, &bus.telegram[7]);
    }
    else if (apci == APCI_RESTART_PDU)
        NVIC_SystemReset();  // Software Reset
    else
        sendAck = T_NACK_PDU;  // Command not supported

    if (sendTel)
        sendAck = T_ACK_PDU;

    if (sendAck)
    {
        d("TX-ACK senderSeqNo=%d\n", senderSeqNo);
        sendConControlTelegram(sendAck, senderSeqNo);
    }
    else
        sendCtrlTelegram[0] = 0;

    if (sendTel)
    {
        sendTelegram[0] = 0xb0 | (bus.telegram[0] & 0x0c); // Control byte
        // 1+2 contain the sender address, which is set by bus.sendTelegram()
        sendTelegram[3] = connectedAddr >> 8;
        sendTelegram[4] = connectedAddr;

        if (sendTelegram[6] & 0x40) // Add the sequence number if applicable
        {
            sendTelegram[6] &= ~0x3c;
            sendTelegram[6] |= connectedSeqNo;
            incConnectedSeqNo = true;
        }
        else
            incConnectedSeqNo = false;
        d("TX-DATA connectedSeqNo=%d\n", connectedSeqNo);
        bus.sendTelegram(sendTelegram, telegramSize(sendTelegram));
    }
}

void BcuUpdate::processConControlTelegram(int tpci)
{
    int senderAddr = (bus.telegram[1] << 8) | bus.telegram[2];

    if (tpci & 0x40)  // An acknowledgement
    {
        tpci &= 0xc3;
        if (tpci == T_ACK_PDU) // A positive acknowledgement
        {
            int curSeqNo = bus.telegram[6] & 0x3c;
            if (incConnectedSeqNo && connectedAddr == senderAddr && lastAckSeqNo !=  curSeqNo)
            {
                d("RX-ACK\n");
                connectedSeqNo += 4;
                connectedSeqNo &= 0x3c;
                incConnectedSeqNo = false;
                lastAckSeqNo = curSeqNo;
            }
        }
        else if (tpci == T_NACK_PDU)  // A negative acknowledgement
        {
            if (connectedAddr == senderAddr)
            {
                d("RX-NACK\n");
                sendConControlTelegram(T_DISCONNECT_PDU, 0);
                connectedAddr = 0;
                incConnectedSeqNo = false;
            }
        }

        incConnectedSeqNo = true;
    }
    else  // A connect/disconnect command
    {
        if (tpci == T_CONNECT_PDU)  // Open a direct data connection
        {
            if (connectedAddr == 0 || connectedAddr == senderAddr)
            {
                connectedTime = HAL_GetTick();
                connectedAddr = senderAddr;
                connectedSeqNo = 0;
                incConnectedSeqNo = false;
                d("RX-CONNECT\n");
                bus.setSendAck(0);

            }
        }
        else if (tpci == T_DISCONNECT_PDU)  // Close the direct data connection
        {
            if (connectedAddr == senderAddr)
            {
                connectedAddr = 0;
                bus.setSendAck(0);
                d("RX-DISCONNECT\n");
            }
        }
    }
}

void BcuUpdate::sendConControlTelegram(int cmd, int senderSeqNo)
{
    if (cmd & 0x40)  // Add the sequence number if the command shall contain it
        cmd |= senderSeqNo & 0x3c;

    sendCtrlTelegram[0] = 0xb0 | (bus.telegram[0] & 0x0c); // Control byte
    // 1+2 contain the sender address, which is set by bus.sendTelegram()
    sendCtrlTelegram[3] = connectedAddr >> 8;
    sendCtrlTelegram[4] = connectedAddr;
    sendCtrlTelegram[5] = 0x60;
    sendCtrlTelegram[6] = cmd;

    bus.sendTelegram(sendCtrlTelegram, 7);
}

