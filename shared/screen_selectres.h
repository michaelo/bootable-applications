#ifndef IMPL_SCREEN_SELECTRES
#define IMPL_SCREEN_SELECTRES

#include "lil_uefi/lil_uefi.h"
#include "utils.h"
#include "draw.h" // for drawRectangle
#include "color.h"
#include "text.h"

static void SelectResolution(EFI_SYSTEM_TABLE *system_table)
{
    EFI_UINTN event;
    EFI_INPUT_KEY key;

    // Set text mode: list all resolutions and prompt user for number
    // Later: find lowest initial resolution and render text on graphic out
    // Problem now is as soon as we start selecting graphics mode we will loose console print to display
    //  Although, this is still testable on qemu where we have tty to terminal
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *text = SetModeText(system_table, 0);

    EFI_UINTN mode_num = 0;
    SetModeGraphics(system_table, mode_num);
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx = GetModeGraphics(system_table);
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *gfx_info;
    EFI_UINTN gfx_info_size;

    EFI_UINT16 scrap[128];
    EFI_UINTN len = 0;

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg = color(255, 255, 255);

    int alive = 1;
    while (alive)
    {
        EFI_UINTN xMargin = 50;
        EFI_UINTN y = 50;

        Bitmap screen = (Bitmap){
            .width = gfx->Mode->info->HorizontalResolution,
            .height = gfx->Mode->info->VerticalResolution,
            .stride = gfx->Mode->info->PixelsPerScanLine,
            .buffer = (void *) gfx->Mode->frame_buffer_base
        };
        
        text->ClearScreen(text);
        drawRectangleToScreen(gfx, 0, 0, 640, 480, color(128,0,128));

        // Write current resolution + help commands
        int x = xMargin;
        FormatterZ(scrap, sizeof(scrap), L"--- Select resolution (ref-box is 640*480) --- mode: %d/%d", mode_num+1, gfx->Mode->max_mode);
        int tmpLen = renderStringFg(&screen, x, y, fg, scrap);
        
        gfx->QueryMode(gfx, mode_num, &gfx_info_size, &gfx_info);

        y += 10;
        FormatterZ(scrap, sizeof(scrap), L"Current resolution: %d x %d", gfx_info->HorizontalResolution, gfx_info->VerticalResolution);
        renderStringFg(&screen, xMargin, y, fg, scrap);

        y += 10;
        y += 10;
        renderStringFg(&screen, xMargin, y, fg, L"Press Left/Right to iterate. Press Enter when happy.");

        

        // Wait for response
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &event);

        // https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html?highlight=waitforkey#efi-simple-text-input-protocol-readkeystroke
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);

        switch (key.ScanCode)
        {
        case EFI_SCAN_Left:
            if (mode_num > 0)
                mode_num -= 1;
            break;
        case EFI_SCAN_Right:
            if (mode_num < gfx->Mode->max_mode - 1)
                mode_num += 1;
            break;
        default:
            switch (key.UnicodeChar)
            {
            case 13:
                alive = 0;
                break;
            default:
                break;
            }
            break;
        };

        SetModeGraphics(system_table, mode_num);
    }
}

#endif