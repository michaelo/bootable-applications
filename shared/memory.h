#ifndef MEMORY_H
#define MEMORY_H

#include "lil_uefi/lil_uefi.h"

EFI_BOOT_SERVICES *boot_services;

static void initialize_memory(EFI_BOOT_SERVICES *boot_services_ptr){
    boot_services = boot_services_ptr;
}

static void * malloc(unsigned long long size)
{
    EFI_STATUS error;
    void * handle;
    error = boot_services->AllocatePool(EFI_MEMORY_TYPE_EfiLoaderData, size, &handle);
    return error ? 0 : handle;
}

static void free(void * ptr)
{
    boot_services->FreePool(ptr);
}

static void * memcpy(void * dst, const void * src, unsigned long long len)
{
    boot_services->CopyMem(dst, (void*)src, len);
    return dst;
}

static void * memset(void * dst, int value, unsigned long long len)
{
    boot_services->SetMem(dst, len, value);
    return dst;
}

#endif