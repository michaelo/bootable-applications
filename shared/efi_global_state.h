#ifndef EFI_GLOBAL_STATE_H
#define EFI_GLOBAL_STATE_H
#include "lil_uefi/lil_uefi.h"

static EFI_SYSTEM_TABLE * efi_global_system_table = 0;

static void efi_initialize_global_state(EFI_SYSTEM_TABLE * system_table)
{
    efi_global_system_table = system_table;
}

static EFI_BOOT_SERVICES * efi_boot_services()
{
    if (efi_global_system_table == 0) return 0;
    return efi_global_system_table->BootServices;
}

#endif
