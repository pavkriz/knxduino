package org.hkfree.knxduino.updater.tests.flashdiff;

public interface DecompressorListener {
    void flashPage(OldWindow oldPagesRam, FlashPage page);
}
