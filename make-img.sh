#!/bin/bash
set -e
set -u

function log() {
    echo "BUILD: $*"
}

FOLDER=$1

FLASHFILE=$FOLDER.img
log Checking for mformat and mmd, if found: create $FLASHFILE

if mformat --version > /dev/null && mmd --version > /dev/null ; then
    log mformat and mmd found, creating $FLASHFILE
    dd if=/dev/zero of=$FLASHFILE bs=1k count=1440
    mformat -i $FLASHFILE -f 1440 ::
    mmd -i $FLASHFILE ::/EFI
    mmd -i $FLASHFILE ::/EFI/BOOT
    mcopy -i $FLASHFILE $FOLDER/EFI/BOOT/BOOTX64.EFI ::/EFI/BOOT
else
    log mformat and/or mmd not found
fi;