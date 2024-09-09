#!/bin/bash
set -e
set -u

SRC_ROOT=$(pwd)
SRC_ENTRY=$1

docker run -v "$SRC_ROOT:/src" uefibuilder $SRC_ENTRY