#!/bin/bash
set -e
set -u

ENTRY_BASE=/src
ENTRY_FILE=$1
BASENAME=$(basename -- "$ENTRY_FILE")
BASENAMENOEXT="${BASENAME%.*}"

echo "UEFI builder"
echo "File: $ENTRY_FILE"

pushd /src
mkdir -p build/EFI/BOOT

# Build
clang $ENTRY_FILE -I. -c -target x86_64-pc-win32-coff -ffreestanding -fno-builtin -fno-stack-protector -fshort-wchar -mno-red-zone -o build/$BASENAMENOEXT.o
lld-link -entry:EfiMain -align:16 -driver -nodefaultlib -dll -subsystem:efi_application -out:build/bootx64.efi build/$BASENAMENOEXT.o


# Prepare EFI folder structure for qemu
cp build/bootx64.efi build/EFI/BOOT/BOOTX64.EFI


# Make image file to flash
FLASHFILE=build/fat.img
dd if=/dev/zero of=$FLASHFILE bs=1k count=1440
mformat -i $FLASHFILE -f 1440 ::
mmd -i $FLASHFILE ::/EFI
mmd -i $FLASHFILE ::/EFI/BOOT
mcopy -i $FLASHFILE build/BOOTX64.EFI ::/EFI/BOOT

popd