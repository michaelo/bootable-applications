#!/bin/bash
set -e
set -u

DEVICE=$2
BASENAME=$(basename -- "$1")

diskutil unmountDisk $DEVICE
sudo dd if=build/$BASENAME.img of=$DEVICE bs=1m 
diskutil eject $DEVICE