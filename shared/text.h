#ifndef IMPL_TEXT
#define IMPL_TEXT

#include "lil_uefi/lil_uefi.h"
#include <stddef.h>
#include <stdarg.h>
#include "shared/utils.h"
#include "shared/bitmap.h"

#ifndef BASIC_TEXT
#include "font8x8/font8x8_latin.h"
#else
#include "font8x8/font8x8_basic.h"
#endif

static int BASE_FONT_SIZE = 8;

static EFI_UINTN IntLen(EFI_INTN value, EFI_UINTN base)
{
    if (value == 0)
        return 1;
    EFI_UINTN len = 0;

    while (value > 0)
    {
        value = value / base;
        len++;
    }

    return len;
}

// capacity: size, excluding e.g. terminating null - must be handled outside.
// base: <=16
// returns number of digits formatted
// TODO: Add variant for EFI_UINTN as well
static EFI_UINTN FormatInt(EFI_UINT16 *buffer, EFI_UINTN capacity, EFI_INTN value, EFI_UINTN base)
{
    if (base > 16)
    {
        return 0;
    }

    static EFI_INT16 charmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    size_t bidx = 0;
    EFI_INTN value_abs = value;
    if (value_abs < 0)
    {
        value_abs *= -1;
        buffer[bidx] = '-';
        bidx += 1;
    }

    EFI_UINTN digits = IntLen(value_abs, base);

    // trunc
    if (digits + bidx > capacity)
    {
        digits = capacity - bidx;
    }

    for (int i = digits - 1; i >= 0; i--)
    {
        EFI_UINTN digit = value_abs % base;
        buffer[bidx + i] = charmap[digit];
        value_abs = value_abs / base;
    }

    return digits + bidx;
}

// capactiy: size, including terminating null.
static EFI_UINTN FormatIntZ(EFI_UINT16 *buffer, EFI_UINTN capacity, EFI_INTN value, EFI_UINTN base)
{
    int len = FormatInt(buffer, capacity - 1, value, base);
    buffer[len] = 0;
    return len;
}

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

static EFI_UINTN StrLen(const EFI_UINT16 *str)
{
    EFI_UINTN len = 0;
    while (str[len] != 0) len++;
    return len;
}

static EFI_UINTN FormatterVZ(EFI_UINT16 *out, EFI_UINTN cap, const EFI_UINT16 *format, va_list args)
{
    typedef enum
    {
        FormatterState_raw,
        FormatterState_escape,
        FormatterState_format
    } FormatterState;

    if (cap == 0)
        return 0;
    
    EFI_UINTN format_idx = 0;
    EFI_UINTN format_len = StrLen(format);
    EFI_UINTN out_idx = 0;
        
    static const EFI_UINTN scratch_size = 64;
    EFI_UINT16 scratch[scratch_size];

    FormatterState state = FormatterState_raw;

    while (format_idx < format_len && out_idx < cap)
    {
        char c = format[format_idx];

        // printf("state: %d, [%lld / %lld] = %c\n", state, format_idx, format_len, c);

        switch (state)
        {
        case FormatterState_raw:
            switch (c)
            {
            case '\\':
                state = FormatterState_escape;
                format_idx += 1;
                break;
            case '%':
                state = FormatterState_format;
                format_idx += 1;
                break;
            default:
                if (out_idx < cap)
                {
                    out[out_idx] = c;
                    out_idx += 1;
                    format_idx += 1;
                }
                break;
            }

            break;
        case FormatterState_escape:
            if (out_idx < cap)
            {
                out[out_idx] = c;
                out_idx += 1;
            }
            state = FormatterState_raw;
            break;
        case FormatterState_format:
            switch (c)
            {
            case 's':
                // va_arg as string/null-terminated buffer
                {
                    EFI_UINT16 *buf_value = va_arg(args, EFI_UINT16 *);
                    size_t buf_len = StrLen(buf_value);

                    // TODO: what to do when reaching end of output buffer
                    for (int i = 0; i < buf_len; i++)
                    {
                        // TODO: can calculate this outside of for and cut limit
                        if (out_idx < cap)
                        {
                            out[out_idx] = buf_value[i];
                            out_idx += 1;
                        }

                    }

                    format_idx += 1;
                    state = FormatterState_raw;
                }
                break;
            // TODO: Add support for unsigned
            case 'd':
                // va_arg as int decimal
                {
                    EFI_INTN int_value = va_arg(args, EFI_INTN);
                    int int_len = FormatInt(scratch, scratch_size, int_value, 10);

                    // Copy formatted int to output
                    // TODO: what to do when reaching end of output buffer
                    for (int i = 0; i < int_len; i++)
                    {
                        // TODO: can calculate this outside of for and cut limit
                        if (out_idx < cap)
                        {
                            out[out_idx] = scratch[i];
                            out_idx += 1;
                        }

                    }

                    format_idx += 1;
                    state = FormatterState_raw;
                }
                break;
            // TODO: add support for unsigned
            case 'x':
                // va_arg as int hexadecimal
                {
                    EFI_INTN int_value = va_arg(args, EFI_INTN);
                    scratch[0] = '0';
                    scratch[1] = 'x';
                    int int_len = FormatInt(&scratch[2], scratch_size - 2, int_value, 16);

                    // Copy formatted int to output
                    // TODO: what to do when reaching end of output buffer
                    for (int i = 0; i < 2+int_len; i++)
                    {
                        // TODO: can calculate this outside of for and cut limit
                        if (out_idx < cap)
                        {
                            out[out_idx] = scratch[i];
                            out_idx += 1;
                        }

                    }

                    format_idx += 1;
                    state = FormatterState_raw;
                }
                break;
            // case 'f':
            //     // va_arg as float
            //     break;
            default:
                // invalid format - print as is
                format_idx += 1;
                state = FormatterState_raw;
                break;
            }
            break;
        default:
            // Invalid state
            break;
        }
    }

    if (out_idx >= cap)
        out_idx = cap - 1;
    out[out_idx] = 0;

    return out_idx;
}

static EFI_UINTN FormatterZ(EFI_UINT16 *out, EFI_UINTN cap, const EFI_UINT16 *format, ...)
{
    va_list args;
    va_start(args, format);
    EFI_UINTN result = FormatterVZ(out, cap, format, args);
    va_end(args);
    return result;
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