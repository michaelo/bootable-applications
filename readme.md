# Bootable applications - an introduction

This repository contains everything that is presented in my "Bootable applications - an introduction" talk.

The intention is to provide a simple method of building UEFI-dependent applications, ready to boot your computer off of.

Now go do something fun!

## Structure

* `/` - entry-point scripts - see the "Getting started"-section below
* `bios/` - contains ovmf.fd, a port of Intels tianocore firmware to use with qemu
* `docker/` - codifies the basic dependencies required to build, link and package UEFI-applications. I do however recommend installing the required tools locally.
* `examples/` - a set of examples consisting of folder pr example with any required .c-files, each exploring UEFI features. Check out examples/hello/hello.c to get started.
* `shared/` - implementations of common, reusable, functionality. Header only library style.
* `lil_uefi/` - a header-only library providing definitions to access UEFI-capabilities
* `presentation/` - exports of the slides as presented

## Getting started

Att! These tests assumes single compilation unit files as entry point, and build/output-directories will be created based on the file basename. For directories of multiple compilation units, see make-based appreoach below.

E.g. ./

1. Install clang and lld, or build the bundled docker image with `./create-docker-image.sh` (will be tagged as 'uefibuilder')
1. Build
    * ... with Docker
        * `./build-with-docker.sh examples/hello/hello.c`
           Will create ./build/hello/EFI/BOOT/BOOTX64.EFI
    * ... natively
        * `./build-natively.sh examples/hello/hello.c`
1. Test with qemu
    * `./run-mount.sh build/hello`
1. Flash to USB
    1. for macos:
        * Insert USB-drive - determine device path, e.g. `diskutil list`
        * `./flashusb-macos.sh build/hello.img /dev/<device>`
    1. for any/other OSes: similar to how you'd flash a USB with other operating system .img-files

## Alternative make-based approach

DIR-argument specifies which folder under "examples" shall be built. All .c-files will then be built and linked.

1. `make DIR=hello`
1. `make DIR=hello run`
1. `make DIR=hello flash`

## Credits

Thanks to [Allen Webster](https://mr4th.com/) and [Ryan Fleury](https://www.rfleury.com/) for writing the [lil_uefi header-only library](https://handmade.network/p/308/lil-uefi/) allowing such convenient way of getting into targeting UEFI, as well as allowing me to vendor the files directly in this repository.

Thanks to [wiki.osdev.org](https://wiki.osdev.org) for vast amounts of great information.

## Relevant reading

* [UEFI specifications](https://uefi.org/specifications)
* [wiki.osdev.org/UEFI](https://wiki.osdev.org/UEFI)


## TODO

* Establish consistent naming rules for shared functionality
* Add support for more architectures besides x86_64
* Add native support for msvc and link.exe