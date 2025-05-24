#ifndef MEMORY_H
#define MEMORY_H

#include "shared/efi_global_state.h"

static void * malloc(unsigned long long size)
{
    EFI_STATUS error;
    void * handle;
    error = efi_boot_services()->AllocatePool(EFI_MEMORY_TYPE_EfiLoaderData, size, &handle);
    return error ? 0 : handle;
}

static void free(void * ptr)
{
    efi_boot_services()->FreePool(ptr);
}

static void * memcpy(void * dst, const void * src, unsigned long long len)
{
    efi_boot_services()->CopyMem(dst, (void*)src, len);
    return dst;
}

static void * memset(void * dst, int value, unsigned long long len)
{
    efi_boot_services()->SetMem(dst, len, value);
    return dst;
}

//Unoptimized memcpy that needs no boot_services pointer
static void * memcpy_naive(void * dst, void * src, unsigned long long len)
{
    char * dst_char = (char*)dst;
    char * src_char = (char*)src;
    for (int i = 0; i < len; i++){
        dst_char[i] = src_char[i];
    }
    return dst;
}

//Unoptimized memset that needs no boot_services pointer
static void * memset_naive(void * dst, int value, unsigned long long len)
{
    unsigned char * dst_char = (unsigned char*)dst;
    unsigned char cvalue = (unsigned char)value;
    for (int i = 0; i < len; i++){
        dst_char[i] = value;
    }
    return dst;
}

#endif