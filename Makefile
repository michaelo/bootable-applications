# Simple Makefile to build and run test-applications.
# Att! Must specify DIR=<subfolder under apps/> to 
#
# Assumptions:
# ...
# Limitations
# ...
# TODO:
# * Add target for creating flashable image and flashing?

ifndef DIR
$(error DIR is not set)
endif

DEBUG=0

BASEDIR=examples
CC=clang
CFLAGS=-Werror -I../.. -I. -DLIL_UEFI_DISABLE_INTEGER_SIZE_CHECKS -c -target x86_64-pc-win32-coff
CL=lld-link
CLFLAGS=-entry:EfiMain -subsystem:efi_application

CCDEBUGFLAGS=-gdwarf
CLDEBUGFLAGS=-debug

# Files specific to example dir to build
FILES=$(wildcard $(BASEDIR)/$(DIR)/*.c)

# Output dirs and files
FLASHFILE=build/$(DIR)/image.img

build/$(DIR)/EFI/BOOT/BOOTX64.EFI: $(FILES) $(wildcard shared/*)
	mkdir -p build/$(DIR)/EFI/BOOT/
	cd build/$(DIR)/ ; $(CC) $(CCDEBUGFLAGS) $(CFLAGS) ../../$(BASEDIR)/$(DIR)/*.c
	cd build/$(DIR)/ ; $(CC) -E $(CFLAGS) ../../$(BASEDIR)/$(DIR)/*.c > $(DIR).o.e
	cd build/$(DIR)/ ; $(CL) $(CLFLAGS) -out:EFI/BOOT/release.BOOTX64.EFI *.o
ifeq (${DEBUG}, 1)
	cd build/$(DIR)/ ; $(CL) $(CLFLAGS) $(CLDEBUGFLAGS) -out:EFI/BOOT/debug.BOOTX64.EFI *.o
endif
#   Choose which .EFI to default load
#   Presumedly the debug sections of the .EFI-file when built for debug is not supported - this is just explorative testing right now
	cd build/$(DIR)/EFI/BOOT ; cp release.BOOTX64.EFI BOOTX64.EFI

run: build/$(DIR)/EFI/BOOT/BOOTX64.EFI
	qemu-system-x86_64 \
		-s -serial stdio \
		-bios ./bios/OVMFx64.fd \
		-drive format=raw,file=fat:rw:build/$(DIR)

# TODO: add checks for availability of commands. Currently dependent on e.g. mtools and diskutil
# Quite possibly abuse of Makefile...
flash: build/$(DIR)/EFI/BOOT/BOOTX64.EFI
ifndef DEVICE
	$(error DEVICE is not set)
endif
	dd if=/dev/zero of=$(FLASHFILE) bs=1k count=1440
	mformat -i $(FLASHFILE) -f 1440 ::
	mmd -i $(FLASHFILE) ::/EFI
	mmd -i $(FLASHFILE) ::/EFI/BOOT
	mcopy -i $(FLASHFILE) build/$(DIR)/EFI/BOOT/BOOTX64.EFI ::/EFI/BOOT

	diskutil unmountDisk $(DEVICE) || true
	sudo dd if=$(FLASHFILE) of=$(DEVICE) bs=1m
	diskutil eject $(DEVICE)

clean:
	rm -rf build
