#include "lil_uefi/lil_uefi.h"
#include <stddef.h>

#include "shared/utils.h"
#include "shared/rand.h"
#include "shared/color.h"
#include "shared/bitmap.h"
#include "shared/draw.h"
#include "shared/text.h"

void plotToBitmap(Bitmap *bitmap, EFI_INT8 *plot, EFI_UINTN plot_width, EFI_UINTN plot_height, Color_BGRA color_fg, Color_BGRA color_bg)
{
    // TBD: Support "CLUT" to map plot from index to color?
    EFI_UINTN width = MIN(plot_width, bitmap->width);
    EFI_UINTN height = MIN(plot_height, bitmap->height);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            bitmap->buffer[bitmap->stride * y + x] = plot[y * plot_width + x] > 0 ? color_fg : color_bg;
        }
    }
}

Bitmap *createPointerBitmap(Memory *mem)
{
    // static EFI_INT8 pointer_plot[8][8] = {
    //     {0, 0, 0, 1, 1, 0, 0, 0},
    //     {0, 0, 0, 1, 1, 0, 0, 0},
    //     {0, 0, 0, 1, 1, 0, 0, 0},
    //     {1, 1, 1, 0, 0, 1, 1, 1},
    //     {1, 1, 1, 0, 0, 1, 1, 1},
    //     {0, 0, 0, 1, 1, 0, 0, 0},
    //     {0, 0, 0, 1, 1, 0, 0, 0},
    //     {0, 0, 0, 1, 1, 0, 0, 0},
    // };
    static EFI_INT8 pointer_plot[8][8] = {
        {1, 1, 1, 1, 1, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 0, 0, 0},
        {1, 0, 0, 1, 1, 1, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 1, 1},
    };
    Bitmap *pointer_bitmap = allocateBitmap(8, 8, mem);
    plotToBitmap(pointer_bitmap, (EFI_INT8 *)pointer_plot, 8, 8, color(255, 255, 255), color(0, 0, 0));
    return pointer_bitmap;
}

#define MAX_SPPS 10
// Att! Does not work for qemu as it does not have pointer drivers
void Pointer(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    // Acquire resources
    Memory memory = initializeMemory(system_table->BootServices);
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx = GetModeGraphics(system_table);
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = system_table->ConOut;
    EFI_SIMPLE_POINTER_PROTOCOL *spp[MAX_SPPS];

#define SCRATCH_SIZE 128
    EFI_UINT16 scratch[SCRATCH_SIZE];

    out->ClearScreen(out);

    EFI_STATUS status = 0;

    // Check for all simple pointer protocols
    EFI_GUID spp_guid = EFI_SIMPLE_POINTER_PROTOCOL_GUID;
    EFI_HANDLE *spp_handle_buf = NULL;
    EFI_UINTN num_spp_handles = 0;
    status = system_table->BootServices->LocateHandleBuffer(EFI_LOCATE_SEARCH_TYPE_ByProtocol, &spp_guid, NULL, &num_spp_handles, &spp_handle_buf);
    num_spp_handles = MIN(num_spp_handles, MAX_SPPS);
    if (status > 0)
    {
        ConsoleWrite(system_table->ConOut, 0, 0, L"error: LocateHandleBuffer");
        // TODO: Cleanup
        return;
    }
    FormatterZ(scratch, SCRATCH_SIZE, L"LocateHandleBuffer found %d pointer protocol handles", num_spp_handles);
    ConsoleWrite(system_table->ConOut, 0, 0, scratch);

    for (int i = 0; i < num_spp_handles; i++)
    {
        status = system_table->BootServices->OpenProtocol(spp_handle_buf[i],
                                                          &spp_guid,
                                                          (void **)&spp[i],
                                                          handle,
                                                          NULL,
                                                          EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
        spp[i]->Reset(spp[i], 1);
    }

    // Set up input event buffer
    EFI_EVENT input_events[MAX_SPPS + 1];
    for (int i = 0; i < num_spp_handles; i++)
        input_events[i] = spp[i]->WaitForInput;
    input_events[num_spp_handles] = system_table->ConIn->WaitForKey;

    EFI_SIMPLE_POINTER_STATE state;

    EFI_UINTN x = gfx->Mode->info->HorizontalResolution / 2;
    EFI_UINTN y = gfx->Mode->info->VerticalResolution / 2;

    Bitmap screen;
    initializeBitmapFromScreenBuffer(&screen, gfx);
    Bitmap *pointer_bitmap = createPointerBitmap(&memory);

    Color_BGRA bg = color(0, 0, 0);
    bg.Reserved = 1;
    Color_BGRA fg = color(255, 255, 255);

    // Initial plot of mouse
    // TODO: Create pointer bitmap once - no need to scale every blt
    float pointer_size = gfx->Mode->info->HorizontalResolution/16;
    pointer_size = MIN(16, pointer_size);
    bltBitmapScaledXor(&screen, pointer_bitmap, x, y, x+pointer_size, y+pointer_size);

    for (;;)
    {
        EFI_UINTN index;
        system_table->BootServices->WaitForEvent(num_spp_handles, input_events, &index);

        spp[index]->GetState(spp[index], &state);

        // Clear area used to print relative movement, then render it
        drawRectangleToScreen(gfx, 0, 0, 400, 40, color(0,0,0));
        FormatterZ(scratch, SCRATCH_SIZE, L"Rel: (%d, %d)", state.RelativeMovementX, state.RelativeMovementY);
        renderString(&screen, 40, 20, bg, fg, 16, scratch);

        // Remove previous bitmap
        if (state.RelativeMovementX != 0 && state.RelativeMovementY != 0)
        {
            bltBitmapScaledXor(&screen, pointer_bitmap, x, y, x+pointer_size, y+pointer_size);

            // Redraw on new position
            x += state.RelativeMovementX;
            y += state.RelativeMovementY;
            bltBitmapScaledXor(&screen, pointer_bitmap, x, y, x+pointer_size, y+pointer_size);
        }
    }

    BlockForKey(system_table, 13);

    memory.free(&memory, pointer_bitmap);
}


// Entry point:
// https://uefi.org/specs/UEFI/2.10/04_EFI_System_Table.html#efi-image-entry-point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    EFI_UINTN event;
    EFI_INPUT_KEY key;
    useFloatingPointMath();

    // Prevent automatic 5min abort
    // https://uefi.org/specs/UEFI/2.10/03_Boot_Manager.html?highlight=minute#load-option-processing
    system_table->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    // For brevity
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = system_table->ConOut;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx = GetModeGraphics(system_table);

    // Let's go!
    Pointer(handle, system_table);

    out->ClearScreen(out);
    ConsoleWrite(out, 0, 0, L"Done! Enter to exit app.");
    BlockForKey(system_table, 13);

    return (0);
}