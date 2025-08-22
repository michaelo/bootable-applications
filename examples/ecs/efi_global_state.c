#include "shared/efi_global_state.h"
#include <stdarg.h>
#include "shared/text.h"

static EFI_SYSTEM_TABLE * efi_global_system_table = 0;

void efi_initialize_global_state(EFI_SYSTEM_TABLE * system_table)
{
    efi_global_system_table = system_table;
}

EFI_SYSTEM_TABLE * efi_system_table()
{
    if (efi_global_system_table == 0) return 0;
    return efi_global_system_table;
}

EFI_BOOT_SERVICES * efi_boot_services()
{
    if (efi_global_system_table == 0) return 0;
    return efi_global_system_table->BootServices;
}

EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL * efi_con_out()
{
    if (efi_global_system_table == 0) return 0;
    return efi_global_system_table->ConOut;
}

void uefi_println(EFI_UINT16 * format, ...)
{
    va_list args;
    va_start(args, format);
    EFI_UINT16 buffer[256];
    EFI_UINTN len = FormatterVZ(buffer, 256, format, args);
    va_end(args);

    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = efi_con_out();

    out->OutputString(out, buffer);
    out->OutputString(out, L"\r\n");
}

void uefi_clear_console()
{
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = efi_con_out();
    out->ClearScreen(out);
    out->SetCursorPosition(out, 0, 0);
}
