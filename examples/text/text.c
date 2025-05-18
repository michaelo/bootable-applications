#include "lil_uefi/lil_uefi.h"
#include <stddef.h>
#include "shared/utils.h"
#include "shared/draw.h"
#include "shared/screen_selectres.h"
#include "shared/text.h"

EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    useFloatingPointMath();
    // Prevent automatic 5min abort
    system_table->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx = GetModeGraphics(system_table);
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = system_table->ConOut;
    out->ClearScreen(out);

    SelectResolution(system_table);

    Bitmap screen;
    initializeBitmapFromScreenBuffer(&screen, gfx);
    
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg = colorTransparent();
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg = color(255, 255, 255);
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL outline = color(127, 0, 127);
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL outline_fg = color(255, 255, 255);

    EFI_UINTN font_size = 8;
    int alive = 1;
    while(alive) {
        EFI_UINTN event_idx;
        EFI_INPUT_KEY key;

        fillScreen(gfx, color(96,0,0));
        drawRectangleToScreen(gfx, 10, 10, 95, 95, color(128, 0, 0));
        drawRectangleToScreen(gfx, 150, 70, 95, 120, color(128, 128, 0));
        drawLineToBitmap(0,0,120,120,&screen, color(255,255,0));

        renderStringOutline(&screen, 100, 100, outline_fg, outline, 2, font_size, L"Woop woop - this is all good! ha æøå da");
        renderString(&screen, 100, 200, bg, fg, font_size, L"Press Left/right to change font size. Enter to shutdown.");

        renderString(&screen, 50, 300, fg, bg, font_size, L"Inverted text");

        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &event_idx);
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);
        switch (key.ScanCode)
        {
        case EFI_SCAN_Left:
            if (font_size > 0)
                font_size -= 1;
            break;
        case EFI_SCAN_Right:
            if (font_size < gfx->Mode->max_mode - 1)
                font_size += 1;
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
    }

    return (0);
}
