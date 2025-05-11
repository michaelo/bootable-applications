#ifndef IMPL_TEXT
#define IMPL_TEXT

#include "lil_uefi/lil_uefi.h"
#include <stddef.h>

#include "font8x8/font8x8_latin.h"

static char *getGlyph(int ord)
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

static void renderChar(Bitmap * bitmap, int dx, int dy, Color_BGRA bg, Color_BGRA fg, int ord)
{
    char * glyph = getGlyph(ord);
    if (!glyph)
        return;

    EFI_UINTN bufferSize = bitmap->height * bitmap->stride;
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            int px = dx + x;
            if (px < 0 || px > bitmap->width) continue;
            int py = dy + y;
            if (py < 0 || py > bitmap->height) continue;
            int set = glyph[y] & 1 << x;
            EFI_UINT64 idx = py * bitmap->stride + px;
            bitmap->buffer[idx] = set ? fg : bg;
        }
    }
}

// Transparent bg
static void renderCharFg(Bitmap * bitmap, int dx, int dy, Color_BGRA fg, int ord)
{
    char * glyph = getGlyph(ord);
    if (!glyph)
        return;

    EFI_UINTN bufferSize = bitmap->height * bitmap->stride;
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            int px = dx + x;
            if (px < 0 || px > bitmap->width) continue;
            int py = dy + y;
            if (py < 0 || py > bitmap->height) continue;
            int set = glyph[y] & 1 << x;
            EFI_UINT64 idx = py * bitmap->stride + px;
            if (set) bitmap->buffer[idx] = fg;
        }
    }
}

// Returns length of string - consider returning final x-coordinat for direct usage
static EFI_UINT64 renderString(Bitmap * bitmap, int dx, int dy, Color_BGRA bg, Color_BGRA fg, EFI_UINT16 *text)
{
    // assumes text is eventually null-terminated
    int cidx = 0;
    while(text[cidx] != 0) {
        renderChar(bitmap, dx+(8*cidx), dy, bg, fg, text[cidx]);
        cidx += 1;
    }
    return cidx * 8;
}

// Returns length of string - consider returning final x-coordinat for direct usage
static EFI_UINT64 renderStringFg(Bitmap * bitmap, int dx, int dy, Color_BGRA fg, EFI_UINT16 *text)
{
    // assumes text is eventually null-terminated
    int cidx = 0;
    while (text[cidx] != 0)
    {
        renderCharFg(bitmap, dx + (8 * cidx), dy, fg, text[cidx]);
        cidx += 1;
    }
    return cidx * 8;
}

#endif