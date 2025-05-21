#!/bin/bash
set -e
set -u

FOLDER=$1

qemu-system-x86_64 -serial stdio -bios ./bios/OVMFx64.fd -drive format=raw,file=fat:rw:$FOLDER
