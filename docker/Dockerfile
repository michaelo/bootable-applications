from ubuntu:latest as base

RUN apt-get update && apt-get -y upgrade
RUN apt-get install -y clang lld
# RUN apt-get install -y git make fasm
# TODO: getting prompted for continent and country... time/locale-stuff?
#
#
RUN apt-get install mtools

# uefi-simple
# RUN apt-get -y install make


# Mount-point for application source
RUN mkdir /src

# Internal base folder for entry point scripts etc
RUN mkdir /app
WORKDIR /app
COPY scripts .
RUN chmod +x /app/*.sh

ENTRYPOINT ["/app/entry.sh"]
