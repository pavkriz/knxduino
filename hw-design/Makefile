TOPDIR=$(PWD)

all:	images

images:
		rm -f knxduino-one/gen/*.png
		eagle -C "SCRIPT $(TOPDIR)/export-board.scr" knxduino-one/knxhome.brd
		eagle -C "SCRIPT $(TOPDIR)/export-schematic.scr" knxduino-one/knxhome.sch