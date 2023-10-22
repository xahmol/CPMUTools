# Path to ctools
# Obatin and get from: https://github.com/mist64/ctools
CTOOLS = /home/xahmol/ctools/bin/

# Application name
SUITE = cpmutools
MOUNT = umount

# Zip file data
README = README.pdf
ZIP = CPMUTools-v091-$(shell date "+%Y%m%d-%H%M").zip
ZIPLIST = $(SUITE).d64 $(SUITE).d71 README.pdf

# Sources for CPMUMount
MOUNTSRC = src/mount.c src/mount_common.c src/ultimate_common_lib.c src/ultimate_dos_lib.c src/ultimate_network_lib.c src/ultimate_time_lib.c

# Hostname of Ultimate II+ target for deployment. Edit for proper IP and usb number
ULTHOST = ftp://192.168.1.19/usb1/cpm/
ULTHOST2 = ftp://192.168.1.31/usb1/cpm/

# Just the usual way to find out if we're
# using cmd.exe to execute make rules.
ifneq ($(shell echo),)
  CMD_EXE = 1
endif

ifdef CMD_EXE
  NULLDEV = nul:
  DEL = -del /f
  RMDIR = rmdir /s /q
else
  NULLDEV = /dev/null
  DEL = $(RM)
  RMDIR = $(RM) -r
endif

########################################

.SUFFIXES:
.PHONY: all clean deploy
all: $(MOUNT).com $(SUITE).d64 $(SUITE).d71 $(ZIP)

# Building  CPMUMount
$(MOUNT).com: $(MOUNTSRC)
	zcc +cpm -lm -DAMALLOC -lc128cpm -o $@ $^

# Building .d64 disk image
$(SUITE).d64: $(MOUNT).com
	$(DEL) $(SUITE).d64 2>$(NULLDEV)
	$(CTOOLS)cformat $(SUITE).d64
	$(CTOOLS)ctools $(SUITE).d64 p $(MOUNT).com

# Building .d71 disk image
$(SUITE).d71: $(MOUNT).com
	$(DEL) $(SUITE).d71 2>$(NULLDEV)
	$(CTOOLS)cformat -2 $(SUITE).d71
	$(CTOOLS)ctools $(SUITE).d71 p $(MOUNT).com

# Creating ZIP file for distribution
$(ZIP): $(ZIPLIST)
	zip $@ $^

clean:
	$(DEL) -f *.bin *.d64 *.d71 *.com *.def 2>$(NULLDEV)

# To deploy software to UII+ enter make deploy. Obviously C128 needs to powered on with UII+ and USB drive connected.
deploy: $(SUITE).d71
	wput -u $(SUITE).d71 $(ULTHOST)
#	wput -u $(SUITE).d71 $(ULTHOST2)
