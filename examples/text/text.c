#include "lil_uefi/lil_uefi.h"
#include <stddef.h>
#include "shared/utils.c"
#include "shared/draw.c"
#include "shared/screen_selectres.c"

#include "font8x8_basic.h"

void renderChar(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx, int dx, int dy, EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg, EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg, int ord)
{
    if (ord < 0 || ord > 127)
        return;

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *frame_buffer_addr = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gfx->Mode->frame_buffer_base;
    EFI_UINT64 frame_buffer_size = gfx->Mode->frame_buffer_size;

    char *bitmap = font8x8_basic[ord];
    int x, y;
    int set;
    int mask;
    for (x = 0; x < 8; x++)
    {
        for (y = 0; y < 8; y++)
        {
            set = bitmap[y] & 1 << x;
            EFI_UINT64 idx = (dy + y) * gfx->Mode->info->HorizontalResolution + (dx + x);
            if(idx >= frame_buffer_size) continue;
            frame_buffer_addr[idx] = set ? fg : bg;
        }
    }
}

void renderString(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx, int dx, int dy, EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg, EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg, char *text)
{
    // assumes text is eventually null-terminated
    int cidx = 0;
    while(text[cidx] != 0) {
        renderChar(gfx, dx+(8*cidx), dy, bg, fg, text[cidx]);
        cidx += 1;
    }
}

EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    EFI_UINTN event;
    EFI_INPUT_KEY key;

    // Prevent automatic 5min abort
    system_table->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx = GetModeGraphics(system_table);

    // For brevity
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = system_table->ConOut;

    out->ClearScreen(out);

    // out->SetCursorPosition(out, 0, 0);
    // out->OutputString(out, L"Hello");

    SelectResolution(system_table);
    drawRectangle(gfx, 10, 10, 10, 10, color(128, 0, 0));

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg = color(0, 0, 0);
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg = color(255, 255, 255);

    // renderChar(gfx, 100, 100, bg, fg, 'A');
    renderString(gfx, 100, 100, bg, fg, "Woop woop - this is all good!");

    BlockForKey(system_table, 13);

    return (0);
}
