#ifndef IMPL_SCREEN_SELECTRES
#define IMPL_SCREEN_SELECTRES

#include "lil_uefi/lil_uefi.h"
#include "utils.c"
#include "draw.c" // for drawRectangle
#include "text.c"

static void SelectResolution(EFI_SYSTEM_TABLE *system_table)
{
    EFI_UINTN event;
    EFI_INPUT_KEY key;

    // Set text mode: list all resolutions and prompt user for number
    // Later: find lowest initial resolution and render text on graphic out
    // Problem now is as soon as we start selecting graphics mode we will loose console print to display
    //  Although, this is still testable on qemu where we have tty to terminal
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *text = SetModeText(system_table, 0);
    // EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = system_table->ConOut;

    EFI_UINTN mode_num = 0;
    SetModeGraphics(system_table, mode_num);
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx = GetModeGraphics(system_table);
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *gfx_info;
    EFI_UINTN gfx_info_size;
    
    int alive = 1;

    EFI_UINT16 scrap[32];
    EFI_UINTN len = 0;

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg = color(255, 255, 255);

    EFI_UINTN xMargin = 50;

    while (alive)
    {
        text->ClearScreen(text);
        drawRectangle(gfx, 0, 0, 640, 480, color(128,0,128));

        // Write current resolution + help commands
        int x = xMargin;
        int tmpLen = renderStringFg(gfx, x, 50, fg, L"--- Select resolution (ref-box is 640*480) ---");

        x += tmpLen;
        FormatIntZ(scrap, sizeof(scrap), mode_num+1);
        tmpLen = renderStringFg(gfx, x, 50, fg, scrap);
        x += tmpLen;
        tmpLen = renderStringFg(gfx, x, 50, fg, L"/");
        x += tmpLen;
        FormatIntZ(scrap, sizeof(scrap), gfx->Mode->max_mode);
        tmpLen = renderStringFg(gfx, x, 50, fg, scrap);

        // ConsoleWrite(text, 0, 0, L"--- Select resolution (ref-box is 640*480) ---");
        gfx->QueryMode(gfx, mode_num, &gfx_info_size, &gfx_info);

        // ConsoleWrite(text, 0, 1, L"Current resolution: ");
        renderStringFg(gfx, xMargin, 60, fg, L"Current resolution: ");

        FormatIntZ(scrap, sizeof(scrap) - 1, gfx_info->HorizontalResolution);

        renderStringFg(gfx, xMargin + renderStringFg(gfx, xMargin, 70, fg, L"Width: "), 70, fg, scrap);



        FormatIntZ(scrap, sizeof(scrap) - 1, gfx_info->VerticalResolution);

        renderStringFg(gfx, xMargin + renderStringFg(gfx, xMargin, 80, fg, L"Height: "), 80, fg, scrap);

        renderStringFg(gfx, xMargin, 100, fg, L"Press Left/Right to iterate. Press Enter when happy.");

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

        // mode_num = uclamp(mode_num, 0, gfx->Mode->max_mode-1);
        SetModeGraphics(system_table, mode_num);
    }
}
#endif