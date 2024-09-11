# Bootable applications - an introduction

This repository contains everything that is presented in my "Bootable applications - an introduction" talk.

The intention is to provide a simple method of building UEFI-dependent applications, ready to boot your computer off of.

Now go do something fun!

## Structure

* `/` - entry-point scripts - see the "Getting started"-section below
* `bios/` - contains ovmf.fd, a port of Intels tianocore firmware to use with qemu
* `docker/` - codifies the basic dependencies required to build, link and package UEFI-applications. I do however recommend installing the required tools locally.
* `examples/` - a set of .c-files, each representing a basic feature of UEFI
* `lil_uefi/` - a header-only library providing definitions to access UEFI-capabilities
* `presentation/` - exports of the slides as presented

## Getting started

1. Install cland and lld, or build the bundled docker image with `./create-docker-image.sh` (will be tagged as 'uefibuilder')
1. Build
    * ... with Docker
        * `./build-with-docker.sh examples/hello.c`
    * ... natively
        * `./build-natively.sh examples/hello.c`
1. Test with qemu
    * `./runmount.sh hello`
1. Flash to USB
    1. for macos:
        * Insert USB-drive - determine device path, e.g. `diskutil list`
        * `./flashusb-macos.sh /dev/\<device> hello`
    1. for any/other OSes: similar to how you'd flash a USB with other operating system .img-files


## Credits

Thanks to [Allen Webster](https://mr4th.com/) and [Ryan Fleury](https://www.rfleury.com/) for writing the [lil_uefi header-only library](https://handmade.network/p/308/lil-uefi/) allowing such convenient way of getting into targeting UEFI, as well as allowing me to vendor the files directly in this repository.

Thanks to [wiki.osdev.org](https://wiki.osdev.org) for vast amounts of great information.

## Relevant reading

* [UEFI specifications](https://uefi.org/specifications)
* [wiki.osdev.org/UEFI](https://wiki.osdev.org/UEFI)
