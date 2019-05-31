package org.hkfree.knxduino.updater.tests.flashdiff;

import java.util.Arrays;

public class Decompressor {

    private enum State {
        EXPECT_COMMAND_BYTE,
        EXPECT_COMMAND_PARAMS,
        EXPECT_RAW_DATA
    }

    private final byte[] cmdBuffer = new byte[5];
    private int expectedCmdLength = 0;
    private int cmdBufferLength = 0;
    private final byte[] scratchpad = new byte[FlashPage.PAGE_SIZE];
    private int scratchpadIndex;
    private int rawLength;
    private State state = State.EXPECT_COMMAND_BYTE;
    private final DecompressorListener listener;
    private final BinImage rom;
    private final OldWindow oldPagesRam = new OldWindow();

    public Decompressor(BinImage rom, DecompressorListener listener) {
        this.rom = rom;
        this.listener = listener;
    }

    private int getLength() {
        if ((cmdBuffer[0] & FlashDiff.FLAG_LONG) == FlashDiff.FLAG_LONG) {
            return ((cmdBuffer[0] & 0b111111) << 8) | (cmdBuffer[1] & 0xff);
        } else {
            return (cmdBuffer[0] & 0b111111);
        }
    }

    private boolean isCopyFromRam() {
        if ((cmdBuffer[0] & FlashDiff.FLAG_LONG) == FlashDiff.FLAG_LONG) {
            return (cmdBuffer[2] & FlashDiff.ADDR_FROM_RAM) == FlashDiff.ADDR_FROM_RAM;
        } else {
            return (cmdBuffer[1] & FlashDiff.ADDR_FROM_RAM) == FlashDiff.ADDR_FROM_RAM;
        }
    }

    private int getCopyAddress() {
        if ((cmdBuffer[0] & FlashDiff.FLAG_LONG) == FlashDiff.FLAG_LONG) {
            return ((cmdBuffer[2] & 0b1111111) << 16) | ((cmdBuffer[3] & 0xff) << 8) | (cmdBuffer[4] & 0xff);
        } else {
            return ((cmdBuffer[1] & 0b1111111) << 16) | ((cmdBuffer[2] & 0xff) << 8) | (cmdBuffer[3] & 0xff);
        }
    }

    private void resetStateMachine() {
        state = State.EXPECT_COMMAND_BYTE;
    }

    public void pageCompleted() {
        listener.flashPage(oldPagesRam, new FlashPage(scratchpad, 0));
        scratchpadIndex = 0;
        Arrays.fill(scratchpad, (byte)0);   // only get get the last (uncomplete) page padded with 0 for unit tests, otherwise not relevant
    }

    public void putByte(byte data) {
        //System.out.println("Decompressor processing new byte " + (data & 0xff) + ", state=" + state);
        switch (state) {
            case EXPECT_COMMAND_BYTE:
                cmdBuffer[0] = data;
                cmdBufferLength = 1;
                expectedCmdLength = 1;
                if ((data & FlashDiff.CMD_COPY) == FlashDiff.CMD_COPY) {
                    expectedCmdLength += 3; // 3 more bytes of source address
                }
                if ((data & FlashDiff.FLAG_LONG) == FlashDiff.FLAG_LONG) {
                    expectedCmdLength += 1; // 1 more byte for longer length
                }
                if (expectedCmdLength > 1) {
                    state = State.EXPECT_COMMAND_PARAMS;
                } else {
                    state = State.EXPECT_RAW_DATA;
                    rawLength = 0;
                }
                break;
            case EXPECT_COMMAND_PARAMS:
                cmdBuffer[cmdBufferLength++] = data;
                if (cmdBufferLength >= expectedCmdLength) {
                    // we have all params of the command
                    if ((cmdBuffer[0] & FlashDiff.CMD_COPY) == FlashDiff.CMD_COPY) {
                        // perform copy
                        if (isCopyFromRam()) {
                            //System.out.println("COPY FROM RAM index=" + scratchpadIndex + " length=" + getLength() + " from addr=" + getCopyAddress());
                            System.arraycopy(oldPagesRam.getOldBinData(), getCopyAddress(), scratchpad, scratchpadIndex, getLength());
                        } else {
                            //System.out.println("COPY FROM ROM index=" + scratchpadIndex + " length=" + getLength() + " from addr=" + getCopyAddress());
                            //System.out.println(rom.getBinData()[getCopyAddress()] & 0xff);
                            //System.out.println(rom.getBinData()[getCopyAddress()+1] & 0xff);
                            //System.out.println(rom.getBinData()[getCopyAddress()+2] & 0xff);
                            System.arraycopy(rom.getBinData(), getCopyAddress(), scratchpad, scratchpadIndex, getLength());
                        }
                        scratchpadIndex += getLength();
                        // and finish command
                        resetStateMachine();
                    } else {
                        // next, read raw data
                        state = State.EXPECT_RAW_DATA;
                        rawLength = 0;
                    }
                } // else expect more params of the command
                break;
            case EXPECT_RAW_DATA:
                // store data read to scratchpad
                scratchpad[scratchpadIndex++] = data;
                rawLength++;
                if (rawLength >= getLength()) {
                    // we have all RAW data, reset state machine
                    resetStateMachine();
                }
        }
    }


}
