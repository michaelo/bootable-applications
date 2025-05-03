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

BASEDIR=examples
CC=clang
CFLAGS=-I../.. -I. -DLIL_UEFI_DISABLE_INTEGER_SIZE_CHECKS -c -target x86_64-pc-win32-coff
CL=lld-link
CLFLAGS=-entry:EfiMain -subsystem:efi_application

FILES=$(wildcard $(BASEDIR)/$(DIR)/*.c)
FLASHFILE=build/$(DIR)/image.img

build/$(DIR)/EFI/BOOT/BOOTX64.EFI: $(FILES)
	mkdir -p build/$(DIR)/EFI/BOOT/
	cd build/$(DIR)/ ; $(CC) $(CFLAGS) ../../$(BASEDIR)/$(DIR)/*.c
	cd build/$(DIR)/ ; $(CL) $(CLFLAGS) -out:EFI/BOOT/BOOTX64.EFI *.o

run: build/$(DIR)/EFI/BOOT/BOOTX64.EFI
	qemu-system-x86_64 -serial stdio -bios ./bios/OVMFx64.fd -drive format=raw,file=fat:rw:build/$(DIR)

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
