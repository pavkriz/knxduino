# selfbus-updater

	Selfbus Updater 0.1

	usage: selfbus-updater.jar [options] <host|port> -fileName <filename.bin> -device <KNX device address>

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
	 -startAddress <hex/dec>  start address in flash memory of selfbus device
	 -appVersionPtr <hex/dev> pointer to APP_VERSION string
	 -uid <hex>               send UID to unlock (default: request UID to unlock)


To be used like this:


	java -jar selfbus-updater.jar -nat <ip address of KNX/IP GW> -fileName "in16-bim112.bin" -appVersionPtr 0x3263 -startAddress 0x2000 -uid 05:B0:01:02:E9:80:AC:AE:E9:07:47:55
