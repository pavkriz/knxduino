# KNXduino-updater

KNXduino Updater 0.3

# Requirements

* JDK 9+
* gradle

# Build

    gradle fatJar
    
*knxduino-updater-1.0-SNAPSHOT-all.jar* file is created in build/libs directory. 

# Usage

	usage: knxduino-updater-1.0-SNAPSHOT-all.jar [options] <host|port> -fileName <filename.hex> -device <KNX device address>

	options:
	 -help -h                 show this help message
	 -version                 show tool/library version and exit
	 -verbose -v              enable verbose status output
	 -localhost <id>          local IP/host name
	 -localport <number>      local UDP port (default system assigned)
	 -port -p <number>        UDP port on <host> (default 3671)
	 -nat -n                  enable Network Address Translation
	 -serial -s               use FT1.2 serial communication
	 -routing                 use KNXnet/IP routing
	 -medium -m <id>          KNX medium [tp0|tp1|p110|p132|rf] (default tp1)
	 -progDevice -p           KNX device address used for programming (default 15.15.192)
	 -device <knxid>          KNX device address in normal operating mode (default none)
	 -appVersionPtr <hex/dev> pointer to APP_VERSION string
	 -uid <hex>               send UID to unlock (default: request UID to unlock)


To be used like this:


	java -jar knxduino-updater-1.0-SNAPSHOT-all.jar -nat <ip address of KNX/IP GW> -fileName "in16-bim112.hex" -appVersionPtr 0x3263 -uid 05:B0:01:02:E9:80:AC:AE:E9:07:47:55
