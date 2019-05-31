package org.hkfree.knxduino.updater.tests.flashdiff;

public class OldWindow {
    public static final int PAGES = 5;
    private byte[] old = new byte[PAGES * FlashPage.PAGE_SIZE];

    public OldWindow() {
    }

    public byte[] getOldBinData() {
        return old;
    }

    public void fillNextPage(byte[] binData, int pageBaseAddress) {
        // shift old data one page back, will leave one "empty" page at the end
        System.arraycopy( old, FlashPage.PAGE_SIZE, old, 0, old.length - FlashPage.PAGE_SIZE );
        // fill the last (current) page (may not be while page when EOF) at the end
        System.arraycopy( binData, pageBaseAddress, old, old.length - FlashPage.PAGE_SIZE , Math.min(FlashPage.PAGE_SIZE, binData.length - pageBaseAddress) );
    }
}
