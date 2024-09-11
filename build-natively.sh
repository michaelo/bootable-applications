#!/bin/bash
set -e
set -u

ENTRY_FILE=$1
BASENAME=$(basename -- "$ENTRY_FILE")
BASENAMENOEXT="${BASENAME%.*}"

echo "UEFI builder"
echo "File: $ENTRY_FILE"

mkdir -p build/$BASENAMENOEXT/EFI/BOOT/

# Build & prepare EFI folder structure for qemu
# clang $ENTRY_FILE -I. -c -target x86_64-pc-win32-coff -Wno-visibility -ffreestanding -fno-builtin -fno-stack-protector -fshort-wchar -mno-red-zone -o build/$BASENAMENOEXT.o
# lld-link -entry:EfiMain -align:16 -driver -nodefaultlib -dll -subsystem:efi_application -out:build/$BASENAMENOEXT/EFI/BOOT/BOOTX64.EFI build/$BASENAMENOEXT.o

# Generate preprocessed file (not necessary - just for insight into what gets compiled)
clang $ENTRY_FILE -E -I. -o build/$BASENAMENOEXT.o.e
# Generate COFF object file
clang $ENTRY_FILE -I. -c -target x86_64-pc-win32-coff -Wno-visibility -o build/$BASENAMENOEXT.o

# Generate PE32+ executable
lld-link -entry:EfiMain -subsystem:efi_application -out:build/$BASENAMENOEXT/EFI/BOOT/BOOTX64.EFI build/$BASENAMENOEXT.o

# Make image file to flash
FLASHFILE=build/$BASENAMENOEXT.img
dd if=/dev/zero of=$FLASHFILE bs=1k count=1440
mformat -i $FLASHFILE -f 1440 ::
mmd -i $FLASHFILE ::/EFI
mmd -i $FLASHFILE ::/EFI/BOOT
mcopy -i $FLASHFILE build/$BASENAMENOEXT/EFI/BOOT/BOOTX64.EFI ::/EFI/BOOT
