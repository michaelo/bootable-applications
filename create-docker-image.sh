#!/bin/bash
set -e
set -u

TAG=uefibuilder
docker build docker -f docker/Dockerfile -t $TAG
echo Created Docker-image \"$TAG\"