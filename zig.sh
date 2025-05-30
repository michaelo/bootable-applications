#!/bin/bash
set -e
set -u

ENTRY_FILE=$1

mkdir -p build/zigtest/EFI/BOOT || true
zig build-exe -O ReleaseFast -target x86_64-uefi-msvc -femit-bin=build/zigtest/EFI/BOOT/bootx64.efi $ENTRY_FILE
qemu-system-x86_64 -serial stdio -bios ./bios/OVMFx64.fd -drive format=raw,file=fat:rw:build/zigtest