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
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *text = SetModeText(system_table, 0);

    EFI_UINTN mode_num = 0;
    SetModeGraphics(system_table, mode_num);
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx = GetModeGraphics(system_table);
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *gfx_info;
    EFI_UINTN gfx_info_size;

    EFI_UINT16 scrap[128];
    EFI_UINTN len = 0;

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg = color(255, 255, 255);
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg = color(0, 0, 0);
    bg.Reserved = 1; // transparent

    int alive = 1;
    Bitmap screen;

    EFI_UINTN font_size = 8;
    while (alive)
    {
        EFI_UINTN xMargin = 50;
        EFI_UINTN y = 50;
        EFI_UINTN line_height = font_size * 1.5;

        initializeBitmapFromScreenBuffer(&screen, gfx);
        
        drawRectangleToScreen(gfx, 0, 0, 640, 480, color(128,0,128));

        // Write current resolution + help commands
        int x = xMargin;
        renderStringF(&screen, x, y, bg, fg, font_size, scrap, sizeof(scrap), L"--- Select resolution (ref-box is 640*480) ---");
        
        gfx->QueryMode(gfx, mode_num, &gfx_info_size, &gfx_info);

        y += line_height;
        y += line_height;
        renderStringF(&screen, xMargin, y, bg, fg, font_size, scrap, sizeof(scrap), L"Current resolution: %d x %d (mode: %d/%d)", gfx_info->HorizontalResolution, gfx_info->VerticalResolution, mode_num+1, gfx->Mode->max_mode);

        y += line_height;
        y += line_height;
        renderString(&screen, xMargin, y, bg, fg, font_size, L"Press Left/Right to iterate. Press Enter when happy.");

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
        case EFI_SCAN_Down:
            if (font_size > 0)
                font_size -= 1;
            break;
        case EFI_SCAN_Up:
            if (font_size < 64)
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

        SetModeGraphics(system_table, mode_num);
    }
}

#endif