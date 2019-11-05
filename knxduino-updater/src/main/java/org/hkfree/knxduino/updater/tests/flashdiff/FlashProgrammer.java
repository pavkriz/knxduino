package org.hkfree.knxduino.updater.tests.flashdiff;

import tuwien.auto.calimero.KNXRemoteException;
import tuwien.auto.calimero.KNXTimeoutException;
import tuwien.auto.calimero.link.KNXLinkClosedException;
import tuwien.auto.calimero.mgmt.KNXDisconnectException;

import java.util.List;

public interface FlashProgrammer {

    void sendCompressedPage(List<Byte> outputDiffStream, long crc32) throws InterruptedException, KNXTimeoutException, KNXLinkClosedException, KNXRemoteException, KNXDisconnectException;
}
