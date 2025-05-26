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
clang $ENTRY_FILE -I. -c -Os -flto -target x86_64-pc-win32-coff -fno-builtin-memset -mno-stack-arg-probe -Werror -o build/$BASENAMENOEXT.o 

# Generate PE32+ executable
log Link object file to PE32+ executable: build/$BASENAMENOEXT/EFI/BOOT/BOOTX64.EFI
lld-link -entry:EfiMain -subsystem:efi_application -out:build/$BASENAMENOEXT/EFI/BOOT/BOOTX64.EFI build/$BASENAMENOEXT.o 

upx -9 build/$BASENAMENOEXT/EFI/BOOT/BOOTX64.EFI

log Finished.