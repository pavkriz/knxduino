package org.hkfree.knxduino.updater.tests.flashdiff;

import java.util.ArrayList;
import java.util.List;

public class FlashDiff {
    private static final int MINIMUM_PATTERN_LENGTH = 6; // less that this is not efficient (metadata would be larger than data)
    private static final int MAX_COPY_LENGTH = 16384-1; // 2^14 = 8 bits + 6 bits (remaining in CMD byte)
    private static final int MAX_LENGTH_SHORT = 64-1;    // 2^6 = 6 bits (remaining in CMD byte)

    public static final int CMD_RAW = 0;
    public static final int CMD_COPY = 0b10000000;
    public static final int FLAG_SHORT = 0;
    public static final int FLAG_LONG = 0b01000000;
    public static final int ADDR_FROM_ROM = 0;
    public static final int ADDR_FROM_RAM = 0b10000000;


    enum SourceType {
        FORWARD_ROM,
        BACKWARD_RAM,
    }

    static class SearchResult {
        int offset;
        int length;
        SourceType sourceType = SourceType.FORWARD_ROM;

        public SearchResult(int logestCandidateSrcOffset, int logestCandidateLength) {
            this.offset = logestCandidateSrcOffset;
            this.length = logestCandidateLength;
        }

        @Override
        public String toString() {
            return "SearchResult{" +
                    "offset=" + offset +
                    ", length=" + length +
                    ", sourceType=" + sourceType +
                    '}';
        }
    }

    public SearchResult letLongestCommonBytes(byte[] ar1, byte ar2[], int patternOffset, int oldDataMinimumAddr, int maxLength) {
        // search as long as possible ar2[beginOffset..n] bytes (pattern) common with ar1[s..t], where n and t are up to length-1 of appropriate arrays and s is unknown
        int logestCandidateSrcOffset = 0;
        int logestCandidateLength = 0;
        for (int i = oldDataMinimumAddr; i < ar1.length; i++) {
            int j = 0;
            while ((patternOffset + j < ar2.length)
                    && (i + j < ar1.length)
                    && (ar1[i + j] == ar2[patternOffset + j])
                    && j < maxLength
            ) {
                j++;
            }
            if (j > logestCandidateLength) {
                // found better (or first) solution
                logestCandidateLength = j;
                logestCandidateSrcOffset = i;
            }
        }
        if (logestCandidateLength > 0) {
            int firstDstPage = patternOffset/FlashPage.PAGE_SIZE;
            int lastDstPage = (patternOffset+logestCandidateLength-1)/FlashPage.PAGE_SIZE;
            if (lastDstPage != firstDstPage) {
                // truncate to a single destination page
                logestCandidateLength = (firstDstPage + 1) * FlashPage.PAGE_SIZE - patternOffset;
            }
//            if (logestCandidateLength >= MINIMUM_PATTERN_LENGTH) {
//                System.out.println(ar1[logestCandidateSrcOffset] & 0xff);
//                System.out.println(ar1[logestCandidateSrcOffset + 1] & 0xff);
//                System.out.println(ar1[logestCandidateSrcOffset + 2] & 0xff);
//            }
            return new SearchResult(logestCandidateSrcOffset, logestCandidateLength);
        } else {
            return new SearchResult(0, 0);
        }
    }

    private int possiblyFinishRawBuffer(List<Byte> rawBuffer, List<Byte> outputDiffStream) {
        int outSize;
        if (!rawBuffer.isEmpty()) {
            byte cmdByte = CMD_RAW;
            if (rawBuffer.size() <= MAX_LENGTH_SHORT) {
                System.out.println("RAW BUFFER SHORT size=" + rawBuffer.size());
                cmdByte = (byte)(cmdByte | FLAG_SHORT);
                cmdByte = (byte)(cmdByte | (rawBuffer.size() & 0b111111));  // command + 6 low bits of the length
                outputDiffStream.add(cmdByte);
                outputDiffStream.addAll(rawBuffer);
                outSize = 1 + rawBuffer.size(); // 1 byte = cmd with short length included
                rawBuffer.clear();
                return outSize;
            } else {
                System.out.println("RAW BUFFER LONG size=" + rawBuffer.size());
                cmdByte = (byte)(cmdByte | FLAG_LONG);
                cmdByte = (byte)(cmdByte | ((rawBuffer.size() >> 8) & 0b111111));  // command + 6 high bits of the length
                byte lengthLowByte = (byte)(rawBuffer.size() & 0xff);           // 8 low bits of the length
                outputDiffStream.add(cmdByte);
                outputDiffStream.add(lengthLowByte);
                outputDiffStream.addAll(rawBuffer);
                outSize = 2 + rawBuffer.size(); // 2 bytes = cmd (6bits of length) + length (8bits of length)
                rawBuffer.clear();
                return outSize;
            }
        } else {
            return 0;
        }
    }

    public void generateDiff(BinImage img1Orig, BinImage img2, FlashProgrammer flashProgrammer) {
        BinImage img1 = new BinImage(img1Orig); // make copy in order to keep img1Orig untouched
        List<Byte> outputDiffStream = new ArrayList<>();
        List<Byte> rawBuffer = new ArrayList<>();
        OldWindow w = new OldWindow();
        int i = 0;
        int size = 0;
        int pages = 0;
        while (i < img2.getBinData().length) {
            SearchResult rBackwardRamWindow = letLongestCommonBytes(w.getOldBinData(), img2.getBinData(), i, 0, MAX_COPY_LENGTH);
            rBackwardRamWindow.sourceType = SourceType.BACKWARD_RAM;
            //SearchResult rBackwardRamWindow = letLongestCommonBytes(img1.getBinData(), img2.getBinData(), i, 0);  // in case we would have two flash banks, ie. full old image available
            int currentPage = i / FlashPage.PAGE_SIZE;
            int firstAddressInThisPage = currentPage * FlashPage.PAGE_SIZE;
            SearchResult rForwardOldFlash = letLongestCommonBytes(img1.getBinData(), img2.getBinData(), i, 0, MAX_COPY_LENGTH);
            rForwardOldFlash.sourceType = SourceType.FORWARD_ROM;
            // which result is better, from FORWARD ROM or BACKWARD RAM?
            SearchResult bestResult = (rForwardOldFlash.length > rBackwardRamWindow.length) ? rForwardOldFlash : rBackwardRamWindow;
            if (bestResult.length >= MINIMUM_PATTERN_LENGTH) {
                size += possiblyFinishRawBuffer(rawBuffer, outputDiffStream);
                System.out.println(String.format("%08x ", i) + bestResult);
                i += bestResult.length;
                size += 5;
                byte cmdByte = (byte)CMD_COPY;
                if (bestResult.length <= MAX_LENGTH_SHORT) {
                    cmdByte = (byte)(cmdByte | FLAG_SHORT | (bestResult.length & 0b111111));  // command + 6 bits of the length
                    outputDiffStream.add(cmdByte);
                } else {
                    cmdByte = (byte)(cmdByte | FLAG_LONG | ((bestResult.length >> 8) & 0b111111));  // command + 6 bits from high byte of the length
                    byte lengthLowByte = (byte)(bestResult.length & 0xff); // 8 low bits of the length
                    outputDiffStream.add(cmdByte);
                    outputDiffStream.add(lengthLowByte);
                }
                // 3 bytes are enough to address ROM or RAM buffer, highest bit indicates ROM or RAM source
                byte addr1 = (byte)(bestResult.offset & 0xff);  // low byte
                byte addr2 = (byte)((bestResult.offset >> 8) & 0xff);  // middle byte
                byte addr3 = (byte)((bestResult.offset >> 16) & 0xff);  // high byte
                int addrFromFlag = bestResult.sourceType == SourceType.BACKWARD_RAM ? ADDR_FROM_RAM : ADDR_FROM_ROM;
                addr3 = (byte)(addr3 | addrFromFlag);
                outputDiffStream.add(addr3);
                outputDiffStream.add(addr2);
                outputDiffStream.add(addr1);
            } else {
                //System.out.println(String.format("%08x RAW: %02x", i, img2.getBinData()[i]));
                rawBuffer.add(img2.getBinData()[i]);
                i++;
            }
            if (i%FlashPage.PAGE_SIZE == 0) {
                // passed to new page
                size += possiblyFinishRawBuffer(rawBuffer, outputDiffStream);
                //p = new FlashPage(img1.getBinData(), i);
                flashProgrammer.sendCompressedPage(outputDiffStream);
                outputDiffStream.clear();
                pages++;
                // emulate we have loaded the original page from ROM to RAM and written new page to ROM
                w.fillNextPage(img1.getBinData(), i - FlashPage.PAGE_SIZE);
                System.arraycopy(img2.getBinData(), i - FlashPage.PAGE_SIZE, img1.getBinData(), i - FlashPage.PAGE_SIZE, FlashPage.PAGE_SIZE);
            }
        }
        size += possiblyFinishRawBuffer(rawBuffer, outputDiffStream);
        if (!outputDiffStream.isEmpty()) {
            flashProgrammer.sendCompressedPage(outputDiffStream);
        }
        //dumpSideBySide(img1, img2);
        System.out.println("Compressed stream length = " + size);
        System.out.println(pages);}
}
