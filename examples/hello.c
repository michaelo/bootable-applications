#include "lil_uefi/lil_uefi.h"
#include <stddef.h>

void memset(void *ptr, int x, size_t n)
{
}

// entry point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    EFI_BOOT_SERVICES *boot_services = system_table->BootServices;

    EFI_EVENT event;
    EFI_INPUT_KEY key;

    // For brevity
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* out = system_table->ConOut;

    out->ClearScreen(out);

    out->SetCursorPosition(out, 0, 0);
    out->OutputString(out, L"Hello,");
    out->SetCursorPosition(out, 0, 1);
    out->OutputString(out, L"   NDC!");

    for(;;) {
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &event);
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);
        if(key.UnicodeChar == 13) break;
    }

    return (0);
}
