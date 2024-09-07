#!/bin/bash
set -e
set -u

DEVICE=$1

diskutil unmountDisk $DEVICE
sudo dd if=build/fat.img of=$DEVICE bs=1m 
diskutil eject $DEVICE