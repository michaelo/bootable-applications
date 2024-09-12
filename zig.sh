#!/bin/bash
set -e
set -u

mkdir -p build/zigtest/EFI/BOOT || true
zig build-exe -target x86_64-uefi-msvc -femit-bin=build/zigtest/EFI/BOOT/bootx64.efi examples/test.zig
qemu-system-x86_64 -serial stdio -bios ./bios/OVMFx64.fd -drive format=raw,file=fat:rw:build/zigtest