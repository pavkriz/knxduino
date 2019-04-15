package de.baycom.selfbus.updater;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.util.HashMap;
import java.util.Map;
import java.util.zip.CRC32;

import tuwien.auto.calimero.IndividualAddress;
import tuwien.auto.calimero.Settings;
import tuwien.auto.calimero.exception.KNXException;
import tuwien.auto.calimero.exception.KNXFormatException;
import tuwien.auto.calimero.exception.KNXIllegalArgumentException;
import tuwien.auto.calimero.knxnetip.KNXnetIPConnection;
import tuwien.auto.calimero.link.KNXNetworkLink;
import tuwien.auto.calimero.link.KNXNetworkLinkFT12;
import tuwien.auto.calimero.link.KNXNetworkLinkIP;
import tuwien.auto.calimero.link.medium.KNXMediumSettings;
import tuwien.auto.calimero.link.medium.PLSettings;
import tuwien.auto.calimero.link.medium.RFSettings;
import tuwien.auto.calimero.link.medium.TPSettings;
import tuwien.auto.calimero.log.LogLevel;
import tuwien.auto.calimero.log.LogManager;
import tuwien.auto.calimero.log.LogService;
import tuwien.auto.calimero.log.LogStreamWriter;
import tuwien.auto.calimero.log.LogWriter;
import tuwien.auto.calimero.mgmt.Destination;
import tuwien.auto.calimero.mgmt.ManagementClient;
import tuwien.auto.calimero.mgmt.ManagementClientImpl;

/**
 * A tool for Calimero updating a Selfbus device in a KNX network.
 * <p>
 * Updater is a {@link Runnable} tool implementation allowing a user to update
 * selbus devices.<br>
 * <br>
 * This tool supports KNX network access using a KNXnet/IP connection or FT1.2
 * connection. It uses the {@link ManagementClient} functionality of the library
 * to read KNX device description, properties, and memory locations. It collects
 * and shows device information similar to the ETS.
 * <p>
 * When running this tool from the console, the <code>main</code>- method of
 * this class is invoked, otherwise use this class in the context appropriate to
 * a {@link Runnable}.<br>
 * In console mode, the KNX device information, as well as errors and problems
 * during its execution are written to <code>System.out</code>.
 *
 * @author Deti Fliegl
 */
public class updater implements Runnable {
    private static final String tool = "Selfbus Updater 0.2";
    private static final String sep = System.getProperty("line.separator");

    private static LogService out = LogManager.getManager().getLogService(
            "tools");

    private final Map options = new HashMap();

    /**
     * Creates a new Updater instance using the supplied options.
     * <p>
     * Mandatory arguments are an IP host/address or a FT1.2 port identifier,
     * depending on the type of connection to the KNX network, and the KNX
     * device individual address ("area.line.device"). See
     * {@link #main(String[])} for the list of options.
     *
     * @param args
     *            list with options
     * @throws KNXIllegalArgumentException
     *             on unknown/invalid options
     */
    public updater(final String[] args) {
        // read in user-supplied command line options
        try {
            parseOptions(args);
        } catch (final KNXIllegalArgumentException e) {
            throw e;
        } catch (final RuntimeException e) {
            throw new KNXIllegalArgumentException(e.getMessage(), e);
        }
    }

    public static void main(final String[] args) {
        final LogWriter w = LogStreamWriter.newUnformatted(LogLevel.WARN,
                System.out, true, false);
        LogManager.getManager().addWriter("", w);
        try {
            final updater d = new updater(args);
            w.setLogLevel(d.options.containsKey("verbose") ? LogLevel.TRACE
                    : LogLevel.WARN);
            final ShutdownHandler sh = new ShutdownHandler().register();
            d.run();
            sh.unregister();
        } catch (final Throwable t) {
            out.log(LogLevel.ERROR, "parsing options", t);
        } finally {
            LogManager.getManager().shutdown(true);
        }
    }

    /**
     * Called by this tool on completion.
     *
     * @param thrown
     *            the thrown exception if operation completed due to a raised
     *            exception, <code>null</code> otherwise
     * @param canceled
     *            whether the operation got canceled before its planned end
     */
    protected void onCompletion(final Exception thrown, final boolean canceled) {
        if (canceled)
            out.info("\nreading device info canceled");
        if (thrown != null)
            out.error("\ncompleted", thrown);
    }

    /**
     * Creates the KNX network link to access the network specified in
     * <code>options</code>.
     * <p>
     *
     * @return the KNX network link
     * @throws KNXException
     *             on problems on link creation
     * @throws InterruptedException
     *             on interrupted thread
     */
    private KNXNetworkLink createLink() throws KNXException,
            InterruptedException {
        final KNXMediumSettings medium = (KNXMediumSettings) options
                .get("medium");
        if (options.containsKey("serial")) {
            // create FT1.2 network link
            final String p = (String) options.get("serial");
            try {
                return new KNXNetworkLinkFT12(Integer.parseInt(p), medium);
            } catch (final NumberFormatException e) {
                return new KNXNetworkLinkFT12(p, medium);
            }
        }
        // create local and remote socket address for network link
        final InetSocketAddress local = createLocalSocket(
                (InetAddress) options.get("localhost"),
                (Integer) options.get("localport"));
        final InetSocketAddress host = new InetSocketAddress(
                (InetAddress) options.get("host"),
                ((Integer) options.get("port")).intValue());
        final int mode = options.containsKey("routing") ? KNXNetworkLinkIP.ROUTING
                : KNXNetworkLinkIP.TUNNELING;
        return new KNXNetworkLinkIP(mode, local, host,
                options.containsKey("nat"), medium);
    }

    /**
     * Reads all command line options, and puts those options into the options
     * map.
     * <p>
     * Default option values are added; on unknown options, a
     * KNXIllegalArgumentException is thrown.
     *
     * @param args
     *            array with command line options
     */
    private void parseOptions(final String[] args) {
        if (args.length == 0)
            return;

        // add defaults
        options.put("port", new Integer(KNXnetIPConnection.DEFAULT_PORT));
        options.put("medium", TPSettings.TP1);

        int i = 0;
        for (; i < args.length; i++) {
            final String arg = args[i];
            if (isOption(arg, "-help", "-h")) {
                options.put("help", null);
                return;
            }
            if (isOption(arg, "-version", null)) {
                options.put("version", null);
                return;
            }
            if (isOption(arg, "-verbose", "-v"))
                options.put("verbose", null);
            else if (isOption(arg, "-localhost", null))
                parseHost(args[++i], true, options);
            else if (isOption(arg, "-localport", null))
                options.put("localport", Integer.decode(args[++i]));
            else if (isOption(arg, "-port", "-p"))
                options.put("port", Integer.decode(args[++i]));
            else if (isOption(arg, "-nat", "-n"))
                options.put("nat", null);
            else if (isOption(arg, "-routing", null))
                options.put("routing", null);
            else if (isOption(arg, "-serial", "-s"))
                options.put("serial", null);
            else if (isOption(arg, "-medium", "-m"))
                options.put("medium", getMedium(args[++i]));
            else if (options.containsKey("serial")
                    && options.get("serial") == null)
                // add argument as serial port number/identifier to serial
                // option
                options.put("serial", arg);
            else if (!options.containsKey("host"))
                // otherwise add a host key with argument as host
                parseHost(arg, false, options);
            else if (isOption(arg, "-progDevice", "-D")) {
                try {
                    options.put("progDevice", new IndividualAddress(args[++i]));
                } catch (final KNXFormatException e) {
                    throw new KNXIllegalArgumentException("KNX device "
                            + e.toString(), e);
                }
            } else if (isOption(arg, "-device", "-d")) {
                try {
                    options.put("device", new IndividualAddress(args[++i]));
                } catch (final KNXFormatException e) {
                    throw new KNXIllegalArgumentException("KNX device "
                            + e.toString(), e);
                }
            } else if (isOption(arg, "-startAddress", "-a"))
                options.put("startAddress", Integer.decode(args[++i]));
            else if (isOption(arg, "-appVersionPtr", ""))
                options.put("appVersionPtr", Integer.decode(args[++i]));
            else if (isOption(arg, "-fileName", "-f"))
                options.put("fileName", args[++i]);
            else if (isOption(arg, "-uid", "-u")) {
                String str = args[++i];
                String[] tokens = str.split(":");

                if (tokens.length == 12) {
                    byte uid[] = new byte[12];
                    for (int n = 0; n < tokens.length; n++) {
                        uid[n] = (byte) Integer.parseUnsignedInt(tokens[n], 16);
                    }
                    options.put("uid", uid);
                }

            } else
                throw new KNXIllegalArgumentException("unknown option " + arg);
        }
        if (options.containsKey("host") == options.containsKey("serial"))
            throw new KNXIllegalArgumentException(
                    "specify either IP host or serial port");
    }

    private static InetSocketAddress createLocalSocket(final InetAddress host,
                                                       final Integer port) {
        final int p = port != null ? port.intValue() : 0;
        try {
            return host != null ? new InetSocketAddress(host, p)
                    : p != 0 ? new InetSocketAddress(
                    InetAddress.getLocalHost(), p) : null;
        } catch (final UnknownHostException e) {
            throw new KNXIllegalArgumentException("failed to get local host "
                    + e.getMessage(), e);
        }
    }

    private static void parseHost(final String host, final boolean local,
                                  final Map options) {
        try {
            options.put(local ? "localhost" : "host",
                    InetAddress.getByName(host));
        } catch (final UnknownHostException e) {
            throw new KNXIllegalArgumentException(
                    "failed to read host " + host, e);
        }
    }

    private static KNXMediumSettings getMedium(final String id) {
        // for now, the local device address is always left 0 in the
        // created medium setting, since there is no user cmd line option for
        // this
        // so KNXnet/IP server will supply address
        if (id.equals("tp0"))
            return TPSettings.TP0;
        else if (id.equals("tp1"))
            return TPSettings.TP1;
        else if (id.equals("p110"))
            return new PLSettings(false);
        else if (id.equals("p132"))
            return new PLSettings(true);
        else if (id.equals("rf"))
            return new RFSettings(null);
        else
            throw new KNXIllegalArgumentException("unknown medium");
    }

    private static boolean isOption(final String arg, final String longOpt,
                                    final String shortOpt) {
        return arg.equals(longOpt) || shortOpt != null && arg.equals(shortOpt);
    }

    private static void showUsage() {
        final StringBuffer sb = new StringBuffer();
        sb.append(tool).append(sep).append(sep);
        sb.append("usage: selfbus-updater.jar")
                .append(" [options] <host|port> -fileName <filename.bin> -device <KNX device address>")
                .append(sep).append(sep);
        sb.append("options:").append(sep);
        sb.append(" -help -h                 show this help message")
                .append(sep);
        sb.append(" -version                 show tool/library version and exit")
                .append(sep);
        sb.append(" -verbose -v              enable verbose status output")
                .append(sep);
        sb.append(" -localhost <id>          local IP/host name").append(sep);
        sb.append(
                " -localport <number>      local UDP port (default system assigned)")
                .append(sep);
        sb.append(" -port -p <number>        UDP port on <host> (default ")
                .append(KNXnetIPConnection.DEFAULT_PORT).append(")")
                .append(sep);
        // sb.append(" -host <id>              remote IP/host name").append(sep);
        sb.append(" -nat -n                  enable Network Address Translation")
                .append(sep);
        sb.append(" -serial -s               use FT1.2 serial communication")
                .append(sep);
        sb.append(" -routing                 use KNXnet/IP routing").append(sep);
        sb.append(
                " -medium -m <id>          KNX medium [tp0|tp1|p110|p132|rf] "
                        + "(default tp1)").append(sep);
        sb.append(
                " -progDevice              KNX device address used for programming (default 15.15.192)")
                .append(sep);
        sb.append(
                " -device <knxid>          KNX device address in normal operating mode (default none)")
                .append(sep);
        sb.append(
                " -startAddress <hex/dec>  start address in flash memory of selfbus device")
                .append(sep);
        sb.append(" -appVersionPtr <hex/dev> pointer to APP_VERSION string")
                .append(sep);
        sb.append(
                " -uid <hex>               send UID to unlock (default: request UID to unlock)")
                .append(sep);
        out.log(LogLevel.ALWAYS, sb.toString(), null);
    }

    private static void showVersion() {
        out.log(LogLevel.ALWAYS, Settings.getLibraryHeader(false), null);
    }

    private static final class ShutdownHandler extends Thread {
        private final Thread t = Thread.currentThread();

        ShutdownHandler register() {
            Runtime.getRuntime().addShutdownHook(this);
            return this;
        }

        void unregister() {
            Runtime.getRuntime().removeShutdownHook(this);
        }

        public void run() {
            t.interrupt();
        }
    }

    public static final int UPD_ERASE_SECTOR = 0;
    public static final int UPD_SEND_DATA = 1;
    public static final int UPD_PROGRAM = 2;
    public static final int UPD_UPDATE_BOOT_DESC = 3;
    public static final int UPD_REQ_DATA = 10;
    public static final int UPD_GET_LAST_ERROR = 20;
    public static final int UPD_SEND_LAST_ERROR = 21;
    public static final int UPD_UNLOCK_DEVICE = 30;
    public static final int UPD_REQUEST_UID = 31;
    public static final int UPD_RESPONSE_UID = 32;
    public static final int UPD_APP_VERSION_REQUEST = 33;
    public static final int UPD_APP_VERSION_RESPONSE = 34;
    public static final int UPD_SET_EMULATION = 100;

    public static final int UDP_UNKONW_COMMAND = 0x100; // <! received command
    // is not defined
    public static final int UDP_CRC_ERROR = 0x101; // <! CRC calculated on the
    // device
    // <! and by the updater don't match
    public static final int UPD_ADDRESS_NOT_ALLOWED_TO_FLASH = 0x102;// <!
    // specifed
    // address
    // cannot
    // be
    // programmed
    public static final int UPD_SECTOR_NOT_ALLOWED_TO_ERASE = 0x103; // <! the
    // specified
    // sector
    // cannot
    // be
    // erased
    public static final int UPD_RAM_BUFFER_OVERFLOW = 0x104; // <! internal
    // buffer for
    // storing the
    // data
    // <! would overflow
    public static final int UPD_WRONG_DESCRIPTOR_BLOCK = 0x105; // <! the boot
    // descriptor
    // block does
    // not exist
    public static final int UPD_APPLICATION_NOT_STARTABLE = 0x106; // <! the
    // programmed
    // application
    // is not
    // startable
    public static final int UPD_DEVICE_LOCKED = 0x107; // <! the device is still
    // locked
    public static final int UPD_UID_MISMATCH = 0x108; // <! UID sent to unlock
    // the device is invalid
    public static final int UDP_NOT_IMPLEMENTED = 0xFFFF; // <! this command is
    // not yet
    // implemented

    public static final int FLASH_SECTOR_SIZE = 4096;

    int streamToInteger(byte[] stream, int offset) {
        return (stream[offset + 3] << 24) | (stream[offset + 2] << 16)
                | (stream[offset + 1] << 8) | stream[offset + 0];
    }

    void integerToStream(byte[] stream, int offset, int val) {
        stream[offset + 3] = (byte) (val >> 24);
        stream[offset + 2] = (byte) (val >> 16);
        stream[offset + 1] = (byte) (val >> 8);
        stream[offset + 0] = (byte) (val);
    }

    int checkResult(byte[] result) {
        return checkResult(result, true);
    }

    int checkResult(byte[] result, boolean verbose) {
        int resultCode = 0;
        if (result[3] == UPD_SEND_LAST_ERROR) {
            resultCode = streamToInteger(result, 4);
            switch (resultCode) {
                case UPD_UID_MISMATCH:
                    System.out.println("failed, UID mismatch.");
                    break;
                case UPD_SECTOR_NOT_ALLOWED_TO_ERASE:
                    System.out.println("Sector not allowed being erased.");
                    break;
                case UPD_RAM_BUFFER_OVERFLOW:
                    System.out.println("RAM Buffer Overflow");
                    break;
                case UDP_CRC_ERROR:
                    System.out.println("CRC error");
                    break;
                case UPD_ADDRESS_NOT_ALLOWED_TO_FLASH:
                    System.out.println("Address not allowed to flash");
                    break;
                case UPD_DEVICE_LOCKED:
                    System.out.println("Device locked");
                    break;
                case UPD_APPLICATION_NOT_STARTABLE:
                    System.out.println("Application not startable");
                    break;
                default:
                    if (resultCode == 0) {
                        if (verbose) {
                            System.out.println("done (" + resultCode + ").");
                        } else
                            System.out.print(".");
                    } else {
                        System.out.println("unknown error (" + resultCode + ").");
                    }
            }
        }
        return resultCode;
    }

    /*
     * (non-Javadoc)
     *
     * @see java.lang.Runnable#run()
     */
    public void run() {
        // ??? as with the other tools, maybe put this into the try block to
        // also call onCompletion
        if (options.isEmpty()) {
            out.log(LogLevel.ALWAYS, tool,
                    null);
            showVersion();
            out.log(LogLevel.ALWAYS, "type -help for help message", null);
            return;
        }
        if (options.containsKey("help")) {
            showUsage();
            return;
        }
        if (options.containsKey("version")) {
            showVersion();
            return;
        }

        Exception thrown = null;
        boolean canceled = false;
        KNXNetworkLink link = null;
        FileInputStream fis = null;

        try {
            ManagementClient mc;
            Destination pd;
            String fName = "";
            int appVersionAddr = 0;
            int startAddress = 0x2000;
            IndividualAddress progDevice = new IndividualAddress(15, 15, 192);
            IndividualAddress device = null;
            byte uid[] = null;
            byte result[] = null;

            if (options.containsKey("fileName")) {
                fName = (String) options.get("fileName");
            }
            if (options.containsKey("appVersionPtr")) {
                appVersionAddr = (int) options.get("appVersionPtr");
            }

            if (options.containsKey("progDevice")) {
                progDevice = (IndividualAddress) options.get("progDevice");
            }
            if (options.containsKey("startAddress")) {
                startAddress = (int) options.get("startAddress");
            }
            if (options.containsKey("device")) {
                device = (IndividualAddress) options.get("device");
            }
            if (options.containsKey("uid")) {
                uid = (byte[]) options.get("uid");
            }

            link = createLink();
            mc = new ManagementClientImpl(link);
            pd = mc.createDestination(progDevice, true);

            if (device != null) {
                Destination d;
                d = mc.createDestination(device, true);
                System.out.println("Restart device in programming mode...");
                try {
                    mc.restart(d, 0, 255);
                } catch (final KNXException e) {
                }
                Thread.sleep(1000);
            }

            if (uid == null) {
                System.out.print("Request UID... ");
                result = mc.sendUpdateData(pd, UPD_REQUEST_UID, new byte[0]);
                if (result.length == 16) {
                    System.out.print("got: ");
                    for (int i = 0; i < result.length - 4; i++) {
                        if (i != 0) {
                            System.out.print(":");
                        }
                        System.out.print(String.format("%02X",
                                result[i + 4] & 0xff));
                    }
                    System.out.println();
                    uid = new byte[12];
                    for (int i = 0; i < result.length - 4; i++) {
                        uid[i] = result[i + 4];
                    }
                } else {
                    System.out.println("Reqest UID failed");
                    mc.restart(pd);
                    throw new RuntimeException("Selfbus udpate failed.");
                }
            }

            System.out.print("Unlock device... ");
            result = mc.sendUpdateData(pd, UPD_UNLOCK_DEVICE, uid);
            if (checkResult(result) != 0) {
                mc.restart(pd);
                throw new RuntimeException("Selfbus udpate failed.");
            }

            fis = new FileInputStream(fName);

            byte[] buf = new byte[12];
            int nRead = 0;
            int total = 0;
            int totalLength = fis.available();
            if (true) {
                int eraseSectors = (totalLength / FLASH_SECTOR_SIZE) + 1;
                int startSector = startAddress / FLASH_SECTOR_SIZE;
                byte[] sector = new byte[1];
                for (int i = 0; i < eraseSectors; i++) {
                    sector[0] = (byte) (i + startSector);
                    System.out.print("Erase sector " + sector[0] + " ...");
                    result = mc.sendUpdateData(pd, UPD_ERASE_SECTOR, sector);
                    if (checkResult(result) != 0) {
                        mc.restart(pd);
                        throw new RuntimeException("Selfbus udpate failed.");
                    }
                }
            }
            CRC32 crc32Block = new CRC32();
            int progSize = 0;
            int progAddress = startAddress;
            boolean doProg = false;
            if (true) {
                System.out.println("Sending application data (" + totalLength
                        + " bytes) ");
                while ((nRead = fis.read(buf)) != -1) {
                    int txSize;
                    int nDone = 0;
                    while (nDone < nRead) {
                        if ((progSize + nRead) > FLASH_SECTOR_SIZE) {
                            txSize = FLASH_SECTOR_SIZE - progSize;
                            doProg = true;
                        } else {
                            txSize = nRead - nDone;
                        }
                        if ((total + nRead) == totalLength) {
                            doProg = true;
                        }
                        if (txSize > 0) {
                            byte[] txBuf = new byte[txSize];

                            for (int i = 0; i < txSize; i++) {
                                txBuf[i] = buf[i + nDone];
                            }
                            crc32Block.update(txBuf, 0, txSize);
                            result = mc
                                    .sendUpdateData(pd, UPD_SEND_DATA, txBuf);
                            if (checkResult(result, false) != 0) {
                                mc.restart(pd);
                                throw new RuntimeException(
                                        "Selfbus udpate failed.");
                            }
                            nDone += txSize;
                            progSize += txSize;
                            total += txSize;
                        }
                        if (doProg) {
                            doProg = false;
                            byte[] progPars = new byte[3 * 4];
                            long crc = crc32Block.getValue();
                            integerToStream(progPars, 0, progSize);
                            integerToStream(progPars, 4, progAddress);
                            integerToStream(progPars, 8, (int) crc);
                            System.out.println();
                            System.out
                                    .print("Program device at flash address 0x"
                                            + Integer.toHexString(progAddress)
                                            + " with " + progSize
                                            + " bytes and CRC32 0x"
                                            + Long.toHexString(crc) + " ... ");
                            result = mc.sendUpdateData(pd, UPD_PROGRAM,
                                    progPars);
                            if (checkResult(result) != 0) {
                                mc.restart(pd);
                                throw new RuntimeException(
                                        "Selfbus udpate failed.");
                            }
                            progAddress += progSize;
                            progSize = 0;
                            crc32Block.reset();
                        }
                    }
                }
                fis.close();
                System.out.println("Wrote " + total
                        + " bytes from file to device.");
                Thread.sleep(1000);
            }
            fis = new FileInputStream(fName);
            totalLength = fis.available();
            CRC32 crc32File = new CRC32();
            byte buffer[] = new byte[totalLength];
            fis.read(buffer);
            fis.close();
            crc32File.update(buffer, 0, totalLength);

            byte bootDescriptor[] = new byte[16];
            int endAddress = startAddress + totalLength;
            integerToStream(bootDescriptor, 0, startAddress);
            integerToStream(bootDescriptor, 4, endAddress);
            integerToStream(bootDescriptor, 8, (int) crc32File.getValue());
            integerToStream(bootDescriptor, 12, appVersionAddr);
            System.out
                    .println("Preparing boot descriptor with start address 0x"
                            + Integer.toHexString(startAddress)
                            + " end address 0x"
                            + Integer.toHexString(endAddress)
                            + " with CRC32 0x"
                            + Long.toHexString(crc32File.getValue())
                            + " APP_VERSION pointer 0x"
                            + Long.toHexString(appVersionAddr) + " ... ");

            int nDone = 0;
            while (nDone < bootDescriptor.length) {
                int txSize = 12;
                int remainBytes = bootDescriptor.length - nDone;
                if (remainBytes < 12) {
                    txSize = remainBytes;
                }
                byte txBuf[] = new byte[txSize];
                for (int i = 0; i < txSize; i++) {
                    txBuf[i] = bootDescriptor[nDone + i];
                }
                result = mc.sendUpdateData(pd, UPD_SEND_DATA, txBuf);
                if (checkResult(result, false) != 0) {
                    mc.restart(pd);
                    throw new RuntimeException("Selfbus udpate failed.");
                }
                nDone += txSize;
            }

            crc32Block.reset();
            crc32Block.update(bootDescriptor);

            byte programBootDescriptor[] = new byte[9];
            integerToStream(programBootDescriptor, 0,
                    bootDescriptor.length);
            integerToStream(programBootDescriptor, 4,
                    (int) crc32Block.getValue());
            System.out.println();
            System.out.print("Update boot descriptor ... ");
            result = mc.sendUpdateData(pd, UPD_UPDATE_BOOT_DESC,
                    programBootDescriptor);
            if (checkResult(result) != 0) {
                mc.restart(pd);
                throw new RuntimeException("Selfbus udpate failed.");
            }
            Thread.sleep(1000);
            System.out.print("Restarting device ... ");
            mc.restart(pd);

        } catch (final KNXException e) {
            thrown = e;
        } catch (final RuntimeException e) {
            thrown = e;
        } catch (final InterruptedException e) {
            canceled = true;
            Thread.currentThread().interrupt();
        } catch (FileNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } finally {
            if (link != null)
                link.close();
            try {
                fis.close();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            onCompletion(thrown, canceled);
        }
    }
}