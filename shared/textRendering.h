#ifndef IMPL_TEXT
#define IMPL_TEXT

#include "lil_uefi/lil_uefi.h"
#include <stddef.h>
#include <stdarg.h>
#include "shared/bitmap.h"
#include "shared/text.h"

#ifndef BASIC_TEXT
#include "font8x8/font8x8_latin.h"
#else
#include "font8x8/font8x8_basic.h"
#endif

static int BASE_FONT_SIZE = 8;


static char *getGlyph(int ord)
{
    if (ord < 0)
        return NULL;

    if (ord <= 0x7F)
    {
        // Contains an 8x8 font map for unicode points U+0000 - U+007F (basic latin)
        return font8x8_basic[ord];
    }
#ifndef BASIC_TEXT
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
#endif
    return NULL;
}

static void renderChar(Bitmap *bitmap, int dx, int dy, Color_BGRA bg, Color_BGRA fg, EFI_UINT16 size, int ord)
{
    char *glyph = getGlyph(ord);
    if (!glyph)
        return;

    float scale = 8 / (float)size;

    for (int x = 0; x < size; x++)
    {
        int px = dx + x;
        if (px < 0 || px >= bitmap->width)
            continue;

        for (int y = 0; y < size; y++)
        {
            int py = dy + y;
            if (py < 0 || py >= bitmap->height)
                continue;

            int set = glyph[(int)(y * scale)] & 1 << (int)(x * scale);
            EFI_UINT64 idx = py * bitmap->stride + px;
            bitmap->buffer[idx] = set
                                      ? fg
                                  : bg.Reserved // transparent if reserved == 0
                                      ? bg
                                      : bitmap->buffer[idx];
        }
    }
}

static void renderCharOutline(Bitmap *bitmap, int dx, int dy, Color_BGRA fg, Color_BGRA outline, EFI_UINT16 outline_size, EFI_UINT16 size, EFI_UINT16 c)
{
    Color_BGRA bg = colorTransparent();

    renderChar(bitmap, dx - outline_size, dy - outline_size, bg, outline, size, c);
    renderChar(bitmap, dx, dy - outline_size, bg, outline, size, c);
    renderChar(bitmap, dx + outline_size, dy - outline_size, bg, outline, size, c);
    renderChar(bitmap, dx - outline_size, dy, bg, outline, size, c);
    renderChar(bitmap, dx + outline_size, dy, bg, outline, size, c);
    renderChar(bitmap, dx - outline_size, dy + outline_size, bg, outline, size, c);
    renderChar(bitmap, dx, dy + outline_size, bg, outline, size, c);
    renderChar(bitmap, dx + outline_size, dy + outline_size, bg, outline, size, c);

    renderChar(bitmap, dx, dy, bg, fg, size, c);
}

// Exploring alternative char/glyph-rendering for optimization purposes
static void renderCharOptimizeTest(Bitmap *bitmap, int dx, int dy, Color_BGRA bg, Color_BGRA fg, EFI_UINT16 size, int ord)
{
    char *glyph = getGlyph(ord);
    if (!glyph)
        return;

    float scale = BASE_FONT_SIZE / (float)size;
    int set, px, py;
    EFI_UINT64 idx;

    // If not transparent
    if (bg.Reserved)
    {
        for (int x = 0; x < size; x++)
        {
            px = dx + x;
            if (px < 0 || px >= bitmap->width)
                continue;

            for (int y = 0; y < size; y++)
            {
                py = dy + y;
                if (py < 0 || py >= bitmap->height)
                    continue;

                set = glyph[(int)(y * scale)] & 1 << (int)(x * scale);
                idx = py * bitmap->stride + px;
                if (!set)
                {
                    bitmap->buffer[idx] = bg;
                }
            }
        }
    }

    // If not transparent
    if (fg.Reserved)
    {
        for (int x = 0; x < size; x++)
        {
            px = dx + x;
            if (px < 0 || px >= bitmap->width)
                continue;

            for (int y = 0; y < size; y++)
            {
                py = dy + y;
                if (py < 0 || py >= bitmap->height)
                    continue;

                set = glyph[(int)(y * scale)] & 1 << (int)(x * scale);
                idx = py * bitmap->stride + px;
                if (set)
                {
                    bitmap->buffer[idx] = fg;
                }
            }
        }
    }
}

// Returns length of string - consider returning final x-coordinat for direct usage
static EFI_UINT64 renderString(Bitmap *bitmap, int dx, int dy, Color_BGRA bg, Color_BGRA fg, EFI_UINT16 size, EFI_UINT16 *text)
{
    // assumes text is eventually null-terminated
    // float scale =
    int cidx = 0;
    while (text[cidx] != 0)
    {
        renderCharOptimizeTest(bitmap, dx + (size * cidx), dy, bg, fg, size, text[cidx]);
        cidx += 1;
    }
    return cidx * size;
}

static EFI_UINT64 renderStringOutline(Bitmap *bitmap, int dx, int dy, Color_BGRA fg, Color_BGRA outline, EFI_UINT16 outline_size, EFI_UINT16 size, EFI_UINT16 *text)
{
    Color_BGRA bg = colorTransparent();

    renderString(bitmap, dx - outline_size, dy - outline_size, bg, outline, size, text);
    renderString(bitmap, dx, dy - outline_size, bg, outline, size, text);
    renderString(bitmap, dx + outline_size, dy - outline_size, bg, outline, size, text);
    renderString(bitmap, dx - outline_size, dy, bg, outline, size, text);
    renderString(bitmap, dx + outline_size, dy, bg, outline, size, text);
    renderString(bitmap, dx - outline_size, dy + outline_size, bg, outline, size, text);
    renderString(bitmap, dx, dy + outline_size, bg, outline, size, text);
    renderString(bitmap, dx + outline_size, dy + outline_size, bg, outline, size, text);

    return renderString(bitmap, dx, dy, bg, fg, size, text);
}

static EFI_UINT64 renderStringF(
    Bitmap *bitmap, int dx, int dy, Color_BGRA bg, Color_BGRA fg, EFI_UINT16 size,
    EFI_UINT16 *scratch, EFI_UINTN scratch_len, const EFI_UINT16 *format, ...)
{

    va_list args;
    va_start(args, format);
    FormatterVZ(scratch, scratch_len, format, args);
    va_end(args);

    return renderString(bitmap, dx, dy, bg, fg, size, scratch);
}

#endif