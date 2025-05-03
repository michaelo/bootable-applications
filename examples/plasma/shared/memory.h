#include "lil_uefi/lil_uefi.h"

EFI_BOOT_SERVICES *boot_services;

void initialize_memory(EFI_BOOT_SERVICES *boot_services_ptr){
    boot_services = boot_services_ptr;
}

void * uefi_malloc(EFI_UINTN poolSize)
{
    EFI_STATUS error;
    void * handle;
    error = boot_services->AllocatePool(EFI_MEMORY_TYPE_EfiLoaderData, poolSize, &handle);
    return error ? 0 : handle;
}

void uefi_free(void * pool)
{
    boot_services->FreePool(pool);
}