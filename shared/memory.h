#ifndef MEMORY_H
#define MEMORY_H

#include "lil_uefi/lil_uefi.h"

typedef struct Memory_S {
    const EFI_BOOT_SERVICES *boot_services;
    void* (*malloc)(struct Memory_S *, unsigned long long);
    void* (*memcpy)(struct Memory_S *, void * dst, const void * src, unsigned long long len);
    void* (*memset)(struct Memory_S *, void * dst, int value, unsigned long long len);
    void (*free)(struct Memory_S *, void * ptr);
} Memory;

static void * memory_h_malloc(Memory * this_, unsigned long long size)
{
    EFI_STATUS error;
    void * handle;
    error = this_->boot_services->AllocatePool(EFI_MEMORY_TYPE_EfiLoaderData, size, &handle);
    return error ? 0 : handle;
}

static void memory_h_free(Memory * this_, void * ptr)
{
    this_->boot_services->FreePool(ptr);
}

static void * memory_h_memcpy(Memory * this_, void * dst, const void * src, unsigned long long len)
{
    this_->boot_services->CopyMem(dst, (void*)src, len);
    return dst;
}

static void * memory_h_memset(Memory * this_, void * dst, int value, unsigned long long len)
{
    this_->boot_services->SetMem(dst, len, value);
    return dst;
}

static Memory initializeMemory(EFI_BOOT_SERVICES *boot_services_ptr)
{
    Memory memory = {
        .boot_services = boot_services_ptr,
        .malloc = memory_h_malloc,
        .memcpy = memory_h_memcpy,
        .memset = memory_h_memset,
        .free = memory_h_free
    };
    return memory;
}

//Unoptimized memcpy that needs no boot_services pointer
static void * memcpy(void * dst, void * src, unsigned long long len)
{
    char * dst_char = (char*)dst;
    char * src_char = (char*)src;
    for (int i = 0; i < len; i++){
        dst_char[i] = src_char[i];
    }
    return dst;
}

//Unoptimized memset that needs no boot_services pointer
static void * memset(void * dst, int value, unsigned long long len)
{
    unsigned char * dst_char = (unsigned char*)dst;
    unsigned char cvalue = (unsigned char)value;
    for (int i = 0; i < len; i++){
        dst_char[i] = value;
    }
    return dst;
}

#endif