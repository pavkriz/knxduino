package org.hkfree.knxduino.updater.tests.flashdiff;

import cz.jaybee.intelhex.Parser;
import cz.jaybee.intelhex.Region;
import cz.jaybee.intelhex.listeners.BinWriter;
import cz.jaybee.intelhex.listeners.RangeDetector;

import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.util.Arrays;

public class BinImage {
    private byte[] binData;
    private long startAddress;
    private long endAddress;

    public BinImage(BinImage source) {
        this.binData = Arrays.copyOf(source.binData, source.binData.length);

        this.startAddress = source.startAddress;
        this.endAddress = source.endAddress;
    }

    public BinImage(BinImage source, int newLength) {
        this.binData = Arrays.copyOf(source.binData, newLength);

        this.startAddress = source.startAddress;
        this.endAddress = source.endAddress;
    }

    private BinImage(byte[] binData, long startAddress, long endAddress) {
        this.binData = binData;
        this.startAddress = startAddress;
        this.endAddress = endAddress;
    }

    public static BinImage readFromHex(String filename) {
        try (FileInputStream hexFis = new FileInputStream(filename)) {
            // init parser
            Parser parser = new Parser(hexFis);

            // 1st iteration - calculate maximum output range
            RangeDetector rangeDetector = new RangeDetector();
            parser.setDataListener(rangeDetector);
            parser.parse();
            hexFis.getChannel().position(0);
            Region outputRegion = rangeDetector.getFullRangeRegion();
            long startAddress = outputRegion.getAddressStart();
            long endAddress = outputRegion.getAddressEnd();

            // 2nd iteration - actual write of the output
            ByteArrayOutputStream os = new ByteArrayOutputStream();
            BinWriter writer = new BinWriter(outputRegion, os, false);
            parser.setDataListener(writer);
            parser.parse();
            byte[] binData = os.toByteArray();
            int totalLength = binData.length;
            System.out.println("Hex file parsed: starting at 0x" + Long.toHexString(startAddress) + ", length " + totalLength
                    + " bytes");
            return new BinImage(binData, startAddress, endAddress);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static BinImage dummyFilled(int size, int fillByte) {
        byte[] binData = new byte[size];
        Arrays.fill(binData, (byte)fillByte);
        return new BinImage(binData, 0, size);
    }

    public byte[] getBinData() {
        return binData;
    }
}
