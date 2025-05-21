// Minimal example showcasing starting a basic UEFI application, printing text to console output, and waiting "Enter" to exit.
#include "lil_uefi/lil_uefi.h"
#include <stddef.h> // for NULL

// Entry point:
// https://uefi.org/specs/UEFI/2.10/04_EFI_System_Table.html#efi-image-entry-point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    EFI_UINTN event;
    EFI_INPUT_KEY key;

    // Prevent automatic 5min abort
    // https://uefi.org/specs/UEFI/2.10/03_Boot_Manager.html?highlight=minute#load-option-processing
    system_table->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    // For brevity
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = system_table->ConOut;

    out->ClearScreen(out);

    // https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html?highlight=setcursorposition#efi-simple-text-output-protocol
    out->SetCursorPosition(out, 0, 0);
    // https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html?highlight=OutputString#efi-simple-text-output-protocol
    out->OutputString(out, L"Hello,");
    out->SetCursorPosition(out, 0, 1);
    out->OutputString(out, L"   you!");

    for (;;)
    {
        // https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html?highlight=waitforevent#id7
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &event);

        // https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html?highlight=waitforkey#efi-simple-text-input-protocol-readkeystroke
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);

        if (key.UnicodeChar == 13)
            break;
    }

    return (0);
}
