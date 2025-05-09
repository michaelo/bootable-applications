#ifndef IMPL_DRAW
#define IMPL_DRAW

#include "lil_uefi/lil_uefi.h"

static void fill(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out, EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixel)
{
    EFI_UINT32 * buffer = (EFI_UINT32 *) &pixel;
    gfx_out->Blt(gfx_out, buffer, EFI_GRAPHICS_OUTPUT_BLT_OPERATION_VideoFill, 0, 0, 0, 0, gfx_out->Mode->info->HorizontalResolution, gfx_out->Mode->info->VerticalResolution, 0);
}

static EFI_GRAPHICS_OUTPUT_BLT_PIXEL color(EFI_UINT32 r, EFI_UINT32 g, EFI_UINT32 b)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL p;
    p.Red = r;
    p.Green = g;
    p.Blue = b;
    return p;
}

static EFI_GRAPHICS_OUTPUT_BLT_PIXEL i32ToColor(int i32color)
{
    EFI_UINT32 color = i32color & 0xffffff;
    return *((EFI_GRAPHICS_OUTPUT_BLT_PIXEL*)&color);
}

static void drawRectangle(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out, EFI_UINT32 dx, EFI_UINT32 dy, EFI_UINT32 width, EFI_UINT32 height, EFI_GRAPHICS_OUTPUT_BLT_PIXEL color)
{
    EFI_UINT32 * buffer = (EFI_UINT32 *) &color;
    gfx_out->Blt(gfx_out, buffer, EFI_GRAPHICS_OUTPUT_BLT_OPERATION_VideoFill, 0, 0, dx, dy, width, height, 0);
}

#endif