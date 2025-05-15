#ifndef IMPL_TEXT
#define IMPL_TEXT

#include "lil_uefi/lil_uefi.h"
#include <stddef.h>
#include <stdarg.h>
#include "shared/utils.h"
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

static EFI_UINTN StrLen(const EFI_UINT16* str) {
    EFI_UINTN len = 0;
    while(str[++len] != 0);
    return len;
}

static EFI_UINTN FormatterZ(EFI_UINT16 *out, EFI_UINTN cap, const EFI_UINT16 *format, ...)
{
    typedef enum
    {
        FormatterState_raw,
        FormatterState_escape,
        FormatterState_format
    } FormatterState;

    if (cap == 0)
        return 0;

    // const size_t scratch_size = 32;
    #define scratch_size 32
    EFI_UINTN format_idx = 0;
    EFI_UINTN format_len = StrLen(format);
    EFI_UINTN out_idx = 0;

    EFI_UINT16 scratch[scratch_size];

    FormatterState state = FormatterState_raw;

    va_list args;
    va_start(args, format);

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
                        }

                        out_idx += 1;
                    }

                    format_idx += 1;
                    state = FormatterState_raw;
                }
                break;
            case 'd':
                // va_arg as int
                {
                    int int_value = va_arg(args, int);
                    int int_len = FormatInt(scratch, scratch_size, int_value, 10);

                    // Copy formatted int to output
                    // TODO: what to do when reaching end of output buffer
                    for (int i = 0; i < int_len; i++)
                    {
                        // TODO: can calculate this outside of for and cut limit
                        if (out_idx < cap)
                        {
                            out[out_idx] = scratch[i];
                        }

                        out_idx += 1;
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

    va_end(args);
    
    if(out_idx >= cap) out_idx = cap-1;
    out[out_idx] = 0;

    return out_idx;
}

#endif