// Testing TIMESTAMP_PROTOCOL - not supported on qemu w OVMF or MBP it seems
// TODO: Check if supported anywhere useful.

#include "lil_uefi/lil_uefi.h"
#include <stddef.h> // for NULL
#include "shared/text.h"
#include "shared/efi_status.h"

// Entry point:
// https://uefi.org/specs/UEFI/2.10/04_EFI_System_Table.html#efi-image-entry-point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    EFI_UINTN event;
    EFI_INPUT_KEY key;
    EFI_STATUS status;

    static const EFI_UINTN scratch_size = 128;
    EFI_CHAR16 scratch[scratch_size];

    // Prevent automatic 5min abort
    // https://uefi.org/specs/UEFI/2.10/03_Boot_Manager.html?highlight=minute#load-option-processing
    system_table->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx = SetModeGraphics(system_table, 0);

    Bitmap screen;
    initializeBitmapFromScreenBuffer(&screen, gfx);
    fillBitmap(&screen, color(0, 0, 0));

    // Get timestamp protocol
    EFI_GUID guid = EFI_TIMESTAMP_PROTOCOL_GUID;
    EFI_TIMESTAMP_PROTOCOL *time = NULL;
    status = system_table->BootServices->LocateProtocol(&guid, 0, (void**)&time);

    if(status != 0) {
        renderStringF(&screen, 10, 10, color(0,0,0), color(255, 255, 255), 8, scratch, scratch_size, L"ERROR: Could not locate timestamp protocol");
        renderStringF(&screen, 10, 20, color(0,0,0), color(255, 255, 255), 8, scratch, scratch_size, L"%x: %s", status, efi_status_str(status));
        goto done;
    }

    // Write timestamp
    EFI_UINT64 ts = time->GetTimestamp();
    renderStringF(&screen, 10, 10, color(0,0,0), color(255, 255, 255), 8, scratch, scratch_size, L"timestamp: %d", ts);

done:
    // Wait for enter
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
