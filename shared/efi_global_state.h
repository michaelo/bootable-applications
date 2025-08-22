#ifndef EFI_GLOBAL_STATE_H
#define EFI_GLOBAL_STATE_H
#include "lil_uefi/lil_uefi.h"

void efi_initialize_global_state(EFI_SYSTEM_TABLE * system_table);
EFI_BOOT_SERVICES * efi_boot_services();
EFI_SYSTEM_TABLE * efi_system_table();
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL * efi_con_out();

void uefi_println(EFI_UINT16 * format, ...);
void uefi_clear_console();
#endif
