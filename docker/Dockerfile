FROM ubuntu:latest as base

RUN apt-get update && apt-get -y upgrade

# Basic required toolchain for build and linkage
RUN apt-get install -y clang lld

# For mformat to create .iso-files
RUN apt-get install -y mtools

# Mount-point for application source
RUN mkdir /src

# Internal base folder for entry point scripts etc
RUN mkdir /app
WORKDIR /app
COPY scripts .
RUN chmod +x /app/*.sh

ENTRYPOINT ["/app/entry.sh"]