#include "lil_uefi/lil_uefi.h"
#include <stddef.h>
#include "shared/utils.c"
#include "shared/draw.c"
#include "shared/screen_selectres.c"

#include "font8x8/font8x8_latin.h"

char *getbitmap(int ord)
{
    if (ord < 0)
        return NULL;

    if (ord <= 0x7F)
    {
        // Contains an 8x8 font map for unicode points U+0000 - U+007F (basic latin)
        return font8x8_basic[ord];
    }
    else if (ord <= 0x9F)
    {
        // Contains an 8x8 font map for unicode points U+0080 - U+009F (C1/C2 control)
        return font8x8_control[ord - 0x80];
    }
    else if (ord <= 0xFF)
    {
        // Contains an 8x8 font map for unicode points U+00A0 - U+00FF (extended latin)
        return font8x8_ext_latin[ord - 0xA0];
    }
    return NULL;
}

void renderChar(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx, int dx, int dy, EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg, EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg, int ord)
{
    if (ord < 0 || ord > 255)
        return;

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *frame_buffer_addr = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gfx->Mode->frame_buffer_base;
    EFI_UINT64 frame_buffer_size = gfx->Mode->frame_buffer_size;

    char *bitmap = getbitmap(ord);
    if (!bitmap)
        return;

    int x, y;
    int set;
    int mask;
    for (x = 0; x < 8; x++)
    {
        for (y = 0; y < 8; y++)
        {
            set = bitmap[y] & 1 << x;
            EFI_UINT64 idx = (dy + y) * gfx->Mode->info->PixelsPerScanLine + (dx + x);
            if (idx >= frame_buffer_size)
                continue;
            frame_buffer_addr[idx] = set ? fg : bg;
        }
    }
}

// Transparent bg
void renderCharFg(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx, int dx, int dy, EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg, int ord)
{
    if (ord < 0 || ord > 255)
        return;

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *frame_buffer_addr = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gfx->Mode->frame_buffer_base;
    EFI_UINT64 frame_buffer_size = gfx->Mode->frame_buffer_size;

    char *bitmap = getbitmap(ord);
    if (!bitmap)
        return;

    int x, y;
    int set;
    int mask;
    for (x = 0; x < 8; x++)
    {
        for (y = 0; y < 8; y++)
        {
            set = bitmap[y] & 1 << x;
            EFI_UINT64 idx = (dy + y) * gfx->Mode->info->PixelsPerScanLine + (dx + x);
            if (idx >= frame_buffer_size)
                continue;
            if (set)
                frame_buffer_addr[idx] = fg;
        }
    }
}

// Returns length of string
EFI_UINT64 renderString(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx, int dx, int dy, EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg, EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg, EFI_UINT16 *text)
{
    // assumes text is eventually null-terminated
    int cidx = 0;
    while(text[cidx] != 0) {
        renderChar(gfx, dx+(8*cidx), dy, bg, fg, text[cidx]);
        cidx += 1;
    }
    return cidx * 8;
}

// Returns length of string
EFI_UINT64 renderStringFg(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx, int dx, int dy, EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg, EFI_UINT16 *text)
{
    // assumes text is eventually null-terminated
    int cidx = 0;
    while (text[cidx] != 0)
    {
        renderCharFg(gfx, dx + (8 * cidx), dy, fg, text[cidx]);
        cidx += 1;
    }
    return cidx * 8;
}

EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    // Prevent automatic 5min abort
    system_table->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx = GetModeGraphics(system_table);
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = system_table->ConOut;
    out->ClearScreen(out);

    SelectResolution(system_table);
    drawRectangle(gfx, 10, 10, 95, 95, color(128, 0, 0));

    drawRectangle(gfx, 150, 70, 95, 120, color(128, 128, 0));

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg = color(0, 0, 0);
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg = color(255, 255, 255);

    // renderChar(gfx, 100, 100, bg, fg, 'A');
    // renderString(gfx, 100, 100, bg, fg, L"Woop woop - this is all good! ha æøå da");
    renderStringFg(gfx, 100, 100, fg, L"Woop woop - this is all good! ha æøå da");

    renderStringFg(gfx, 100, 200, fg, L"Press Enter to shutdown");

    BlockForKey(system_table, 13);
    system_table->RuntimeServices->ResetSystem(EFI_RESET_TYPE_Shutdown, 0, 0, NULL);

    return (0);
}
