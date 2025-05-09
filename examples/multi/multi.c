#include "lil_uefi/lil_uefi.h"
#include <stddef.h>

#include "shared/utils.h"
#include "shared/rand.h"
#include "shared/draw.h"

#include "shared/screen_selectres.h" // For SelectResolution()

// typedef struct
// {
//     EFI_HANDLE handle;
//     EFI_SYSTEM_TABLE *system_table;
// } Context;

void RandomFill(EFI_SYSTEM_TABLE* system_table)
{
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
            
        fill(gfx_out_prot, i32ToColor(rand()));
        // ConsoleWrite(system_table->ConOut, 0, 0, L"Enter to exit. Any other key to fill random color.");
    }
}

// void Menu(Context ctx)
// {
//     EFI_UINTN event;
//     EFI_INPUT_KEY key;

//     // Set text mode
//     EFI_GUID gfx_out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
//     struct EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out_prot;

//     EFI_STATUS status;
//     status = ctx.system_table->BootServices->LocateProtocol(&gfx_out_guid, 0, (void **)&gfx_out_prot);

//     for (;;)
//     {
//         ctx.system_table->BootServices->WaitForEvent(1, &ctx.system_table->ConIn->WaitForKey, &event);

//         // https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html?highlight=waitforkey#efi-simple-text-input-protocol-readkeystroke
//         ctx.system_table->ConIn->ReadKeyStroke(ctx.system_table->ConIn, &key);

//         if (key.UnicodeChar == 13)
//             break;
//     }
// }

// void DrawText(Context ctx, Pos, char* string)
// {

// }

// Entry point:
// https://uefi.org/specs/UEFI/2.10/04_EFI_System_Table.html#efi-image-entry-point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    EFI_UINTN event;
    EFI_INPUT_KEY key;

    // Context ctx = {
    //     .handle = handle,
    //     .system_table = system_table,
    // };

    // Prevent automatic 5min abort
    // https://uefi.org/specs/UEFI/2.10/03_Boot_Manager.html?highlight=minute#load-option-processing
    system_table->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    // For brevity
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = system_table->ConOut;

    out->ClearScreen(out);

    ConsoleWrite(out, 0, 0, L"Hello,");
    ConsoleWrite(out, 0, 1, L"   UEFI!");

    // Let's go!
    SelectResolution(system_table);
    RandomFill(system_table);
    // Menu(ctx);

    out->ClearScreen(out);
    ConsoleWrite(out, 0, 0, L"Done! Enter to exit app.");
    BlockForKey(system_table, 13);

    return (0);
}
