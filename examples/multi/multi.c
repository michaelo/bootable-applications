#include "lil_uefi/lil_uefi.h"
#include <stddef.h>

#include "shared/utils.h"
#include "shared/rand.h"
#include "shared/color.h"
#include "shared/draw.h"

#include "shared/screen_selectres.h" // For SelectResolution()

// int do_halt = 1;
void RandomFill(EFI_SYSTEM_TABLE* system_table)
{
    // while (do_halt) {
    //     asm volatile ("pause");
    // }
    
    // Assume we're inn graphics mode?
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out_prot = GetModeGraphics(system_table);
    ConsoleWrite(system_table->ConOut, 0, 0, L"Enter to exit. Any other key to fill random color.");

    for(;;) {
        EFI_UINTN event;
        EFI_INPUT_KEY key;

        // TODO: Read key non-blocking?
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &event);
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);
        if (key.UnicodeChar == 13)
            break;
            
        fillScreen(gfx_out_prot, i32ToColor(rand()));
    }
}

void PrintImageAddress(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table) {
    EFI_LOADED_IMAGE_PROTOCOL *loaded_image = NULL;
    EFI_GUID guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

    system_table->BootServices->HandleProtocol(handle, &guid, (void**)&loaded_image);

    EFI_UINT16 scrap[32];
    EFI_UINTN len = 0;

    system_table->ConOut->OutputString(system_table->ConOut, L"Image base: ");

    len = FormatIntZ(scrap, 32, (EFI_UINTN)loaded_image->ImageBase, 16);
    system_table->ConOut->OutputString(system_table->ConOut, scrap);

    // 108617728
    // 0x6796000
}

// Entry point:
// https://uefi.org/specs/UEFI/2.10/04_EFI_System_Table.html#efi-image-entry-point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    initialize_memory(system_table->BootServices);

    EFI_UINTN event;
    EFI_INPUT_KEY key;

    // Prevent automatic 5min abort
    // https://uefi.org/specs/UEFI/2.10/03_Boot_Manager.html?highlight=minute#load-option-processing
    system_table->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    // For brevity
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = system_table->ConOut;

    // out->ClearScreen(out);

    // Let's go!
    ConsoleWrite(out, 0, 0, L"Step 1");
    // PrintImageAddress(handle, system_table);
    ConsoleWrite(out, 0, 0, L"Step 2");
    SelectResolution(system_table);
    ConsoleWrite(out, 0, 0, L"Step 3");
    RandomFill(system_table);
    // Menu(ctx);

    out->ClearScreen(out);
    ConsoleWrite(out, 0, 0, L"Done! Enter to exit app.");
    BlockForKey(system_table, 13);

    return (0);
}
