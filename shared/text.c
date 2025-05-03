#ifndef IMPL_TEXT
#define IMPL_TEXT

#include "lil_uefi/lil_uefi.h"
#include <stddef.h>

#include "font8x8/font8x8_latin.h"

static char *getbitmap(int ord)
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

static void renderChar(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx, int dx, int dy, EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg, EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg, int ord)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *frame_buffer_addr = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gfx->Mode->frame_buffer_base;
    EFI_UINT64 frame_buffer_size = gfx->Mode->frame_buffer_size;

    char *bitmap = getbitmap(ord);
    if (!bitmap)
        return;

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            int set = bitmap[y] & 1 << x;
            EFI_UINT64 idx = (dy + y) * gfx->Mode->info->PixelsPerScanLine + (dx + x);
            if (idx >= frame_buffer_size)
                continue;
            frame_buffer_addr[idx] = set ? fg : bg;
        }
    }
}

// Transparent bg
static void renderCharFg(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx, int dx, int dy, EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg, int ord)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *frame_buffer_addr = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gfx->Mode->frame_buffer_base;
    EFI_UINT64 frame_buffer_size = gfx->Mode->frame_buffer_size;

    char *bitmap = getbitmap(ord);
    if (!bitmap)
        return;

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            int set = bitmap[y] & 1 << x;
            EFI_UINT64 idx = (dy + y) * gfx->Mode->info->PixelsPerScanLine + (dx + x);
            if (idx >= frame_buffer_size)
                continue;
            if (set)
                frame_buffer_addr[idx] = fg;
        }
    }
}

// Returns length of string - consider returning final x-coordinat for direct usage
static EFI_UINT64 renderString(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx, int dx, int dy, EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg, EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg, EFI_UINT16 *text)
{
    // assumes text is eventually null-terminated
    int cidx = 0;
    while(text[cidx] != 0) {
        renderChar(gfx, dx+(8*cidx), dy, bg, fg, text[cidx]);
        cidx += 1;
    }
    return cidx * 8;
}

// Returns length of string - consider returning final x-coordinat for direct usage
static EFI_UINT64 renderStringFg(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx, int dx, int dy, EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg, EFI_UINT16 *text)
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

#endif