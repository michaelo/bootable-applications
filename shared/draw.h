#ifndef IMPL_DRAW
#define IMPL_DRAW

#include "lil_uefi/lil_uefi.h"
#include "shared/bitmap.h"

static void fillScreen(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out, EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixel)
{
    EFI_UINT32 * buffer = (EFI_UINT32 *) &pixel;
    gfx_out->Blt(gfx_out, buffer, EFI_GRAPHICS_OUTPUT_BLT_OPERATION_VideoFill, 0, 0, 0, 0, gfx_out->Mode->info->HorizontalResolution, gfx_out->Mode->info->VerticalResolution, 0);
}

static void drawRectangleToScreen(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out, EFI_UINT32 dx, EFI_UINT32 dy, EFI_UINT32 width, EFI_UINT32 height, EFI_GRAPHICS_OUTPUT_BLT_PIXEL color)
{
    EFI_UINT32 * buffer = (EFI_UINT32 *) &color;
    gfx_out->Blt(gfx_out, buffer, EFI_GRAPHICS_OUTPUT_BLT_OPERATION_VideoFill, 0, 0, dx, dy, width, height, 0);
}

static void drawBitmapToScreen(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out, EFI_UINT32 dx, EFI_UINT32 dy, Bitmap * sprite)
{
    gfx_out->Blt(gfx_out, (EFI_UINT32*)sprite->buffer, EFI_GRAPHICS_OUTPUT_BLT_OPERATION_BufferToVideo, 0, 0, dx, dy, sprite->width, sprite->height, sprite->stride * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
}

static void grabScreenToBitmap(EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out, EFI_UINT32 sx, EFI_UINT32 sy, Bitmap * sprite)
{
    gfx_out->Blt(gfx_out, (EFI_UINT32*)sprite->buffer, EFI_GRAPHICS_OUTPUT_BLT_OPERATION_VideoToBuffer, sx, sy, 0, 0, sprite->width, sprite->height, sprite->stride  * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
}

#endif
