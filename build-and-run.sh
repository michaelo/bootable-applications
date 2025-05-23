#!/bin/bash
BASENAME=$(basename -- $1)
BASENAMENOEXT="${BASENAME%%.*}"
# echo $BASENAMENOEXT
./build-natively.sh $1
./run-mount.sh build/$BASENAMENOEXT