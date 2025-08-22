#ifndef IMPL_TEXT_FORMATTING
#define IMPL_TEXT_FORMATTING

#include "lil_uefi/lil_uefi.h"
#include <stddef.h>
#include <stdarg.h>
#include "shared/utils.h"

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
                    EFI_INT32 int_value = va_arg(args, EFI_INT32);
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
            case 'f':
                // va_arg as float
                {
                    double double_value = va_arg(args, double);
                    //convert double to string:
                    EFI_UINTN int_part = (EFI_UINTN)double_value;
                    double frac_part = double_value - (double)int_part;
                    EFI_UINTN int_len = FormatInt(scratch, scratch_size, int_part, 10);

                    scratch[int_len] = '.';
                    EFI_UINTN frac_len = 0;
                    if (frac_part > 0)
                    {
                        // convert fractional part to string
                        frac_part *= 10;
                        while (frac_len < 6 && frac_part > 0)
                        {
                            EFI_UINTN digit = (EFI_UINTN)frac_part;
                            scratch[int_len + 1 + frac_len] = '0' + digit;
                            frac_part -= digit;
                            frac_part *= 10;
                            frac_len += 1;
                        }
                    }
                    else
                    {
                        scratch[int_len + 1] = '0';
                        frac_len = 1;
                    }

                    // Copy formatted double to output
                    // TODO: what to do when reaching end of output buffer
                    for (int i = 0; i < int_len + 1 + frac_len; i++)
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

#endif