package org.hkfree.knxduino.updater.tests.flashdiff;

import java.util.List;

public interface FlashProgrammer {

    void sendCompressedPage(List<Byte> outputDiffStream);
}
