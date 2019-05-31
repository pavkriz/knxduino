package org.hkfree.knxduino.updater.tests.flashdiff;

import java.util.Arrays;

public class FlashPage {
    public static final int PAGE_SIZE = 2048;
    private byte[] old; // old content before patching

    public FlashPage(byte[] fromByteArray, int begin) {
        old = Arrays.copyOfRange(fromByteArray, begin, begin + PAGE_SIZE);
    }

    public byte[] getOldBinData() {
        return old;
    }
}
