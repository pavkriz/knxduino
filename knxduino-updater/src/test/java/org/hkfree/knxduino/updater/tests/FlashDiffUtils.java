package org.hkfree.knxduino.updater.tests;

import org.hkfree.knxduino.updater.tests.flashdiff.BinImage;
import org.junit.Assert;

public class FlashDiffUtils {

    public static boolean isEqual(byte[] a1, byte[] a2, int begin, int size) {
        for(int i=begin;i<begin+size;i++)
            if (a1[i] != a2[i])
                return false;
        return true;
    }

    public static void dumpSideBySide(BinImage img1, BinImage img2) {
        byte[] ar1 = img1.getBinData();
        byte[] ar2 = img2.getBinData();
        dumpSideBySide(ar1, ar2, 0);
    }

    public static void dumpSideBySide(byte[] ar1, byte[] ar2, int offset) {
        for (int i = offset; i < offset+2048; i+=8) {
            System.out.print(String.format("%04x:  %02x %02x %02x %02x %02x %02x %02x %02x | %02x %02x %02x %02x %02x %02x %02x %02x",
                    i,
                    ar1[i], ar1[i+1], ar1[i+2], ar1[i+3], ar1[i+4], ar1[i+5], ar1[i+6], ar1[i+7],
                    ar2[i], ar2[i+1], ar2[i+2], ar2[i+3], ar2[i+4], ar2[i+5], ar2[i+6], ar2[i+7]
            ));
            if (isEqual(ar1, ar2, i, 8)) {
                System.out.println(" EQ");
            } else {
                System.out.println(" NOT EQUAL !!!");
                Assert.fail("Arrays not equals");
            }
        }
    }

}
