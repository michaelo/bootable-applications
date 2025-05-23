#!/bin/bash
set -e
set -u

function log() {
    echo "BUILD: $*"
}

ENTRY_FILE=$1
BASENAME=$(basename -- "$ENTRY_FILE")
BASENAMENOEXT="${BASENAME%.*}"

log "UEFI builder"
log "File: $ENTRY_FILE"

# Build & prepare EFI folder structure for qemu
mkdir -p build/$BASENAMENOEXT/EFI/BOOT/

# Generate COFF object file
log Compile COFF object file: build/$BASENAMENOEXT.o
clang $ENTRY_FILE -I. -c -target x86_64-pc-win32-coff -fno-builtin-memset -mno-stack-arg-probe -Werror -o build/$BASENAMENOEXT.o 

# Generate PE32+ executable
log Link object file to PE32+ executable: build/$BASENAMENOEXT/EFI/BOOT/BOOTX64.EFI
lld-link -entry:EfiMain -subsystem:efi_application -out:build/$BASENAMENOEXT/EFI/BOOT/BOOTX64.EFI build/$BASENAMENOEXT.o 

# Make image file to flash - move to separate file?
FLASHFILE=build/$BASENAMENOEXT.img
log Checking for mformat and mmd, if found: create $FLASHFILE
if mformat --version > /dev/null && mmd --version > /dev/null ; then
    log mformat and mmd found, creating $FLASHFILE
    dd if=/dev/zero of=$FLASHFILE bs=1k count=1440
    mformat -i $FLASHFILE -f 1440 ::
    mmd -i $FLASHFILE ::/EFI
    mmd -i $FLASHFILE ::/EFI/BOOT
    mcopy -i $FLASHFILE build/$BASENAMENOEXT/EFI/BOOT/BOOTX64.EFI ::/EFI/BOOT
else
    log mformat and/or mmd not found
fi;

log Finished.