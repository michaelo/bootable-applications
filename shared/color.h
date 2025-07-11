#ifndef COLOR_H
#define COLOR_H

#include "lil_uefi/lil_uefi.h"

typedef EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color_BGRA;

typedef union {
    Color_BGRA asColor;
    unsigned int asInt;
    float asFloat;
} Pixel;

static Color_BGRA color(EFI_UINT32 r, EFI_UINT32 g, EFI_UINT32 b)
{
    return (Color_BGRA) {.Blue = b, .Green = g, .Red = r, .Reserved = 255};
}

static Color_BGRA colorTransparent()
{
        return (Color_BGRA) {.Blue = 0, .Green = 0, .Red = 0, .Reserved = 0};
}

static Color_BGRA i32ToColor(int i32color)
{
    EFI_UINT32 color = i32color & 0xffffff;
    return *((Color_BGRA*)&color);
}

#endif
