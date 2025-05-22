#ifndef BITMAP_H
#define BITMAP_H

#include "shared/math.h"
#include "shared/color.h"
#include "shared/memory.h"

typedef struct Bitmap {
    EFI_UINT32 width;
    EFI_UINT32 height;
    EFI_UINT32 stride;
    Color_BGRA * buffer;
} Bitmap;

static Bitmap * allocateBitmap(EFI_UINT32 width, EFI_UINT32 height, Memory * memory)
{
    Bitmap * bitmap = (Bitmap*) memory->malloc(memory, sizeof(Bitmap) + sizeof(Color_BGRA) * width * height);
    bitmap->width = width;
    bitmap->height = height;
    bitmap->stride = width;
    bitmap->buffer = (Color_BGRA *) (&(bitmap->buffer) + sizeof(Color_BGRA *));
    return bitmap;
}

static Bitmap * initializeBitmapFromBuffer(Bitmap * bitmap, EFI_UINT32 width, EFI_UINT32 height, EFI_UINT32 stride, const unsigned int * buffer)
{
    *bitmap = (Bitmap) {
        .width = width,
        .height = height,
        .stride = stride,
        .buffer = (Color_BGRA*)buffer       
    };
    return bitmap;
}

static Bitmap * initializeBitmapFromScreenBuffer(Bitmap * bitmap, EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out)
{
    return initializeBitmapFromBuffer(
        bitmap,
        gfx_out->Mode->info->HorizontalResolution, 
        gfx_out->Mode->info->VerticalResolution, 
        gfx_out->Mode->info->PixelsPerScanLine, 
        (const unsigned int *) gfx_out->Mode->frame_buffer_base
    );
}

static void fillBitmap(Bitmap * bitmap, Color_BGRA color)
{
    EFI_UINTN numpixels = bitmap->stride * bitmap->height;
    for (EFI_UINT64 idx = 0; idx < numpixels; idx += 1)
    {
        bitmap->buffer[idx] = color;
    }
}

static void drawBitmapTransparent(EFI_UINT32 dx, EFI_UINT32 dy, Bitmap * source, Bitmap * target)
{
    for (EFI_UINT32 y = 0; y < source->height; y++)
    {
        int py = y + dy;
        if (py < 0 || py >= target->height) continue;
        for (EFI_UINT32 x = 0; x < source->width; x++)
        {
            int px = x + dx;
            if (px < 0 || px >= target->width) continue;
            Color_BGRA pixel = source->buffer[y * source->width + x];
            if (pixel.Reserved != 0) {
                target->buffer[py * target->stride + px] = pixel;
            }
        }
    }
}

static void drawLine(int x0, int y0, int x1, int y1, Bitmap * target, Color_BGRA color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    EFI_UINT32 xSteps = abs(dx);
    EFI_UINT32 ySteps = abs(dy);
    EFI_UINT32 numSteps = MAX(xSteps, ySteps);
    float numStepsF = (float) numSteps;
    float xIncrement = dx / numStepsF;
    float yIncrement = dy / numStepsF;

    float xf = x0;
    float yf = y0;
    for (EFI_UINT32 step = 0; step <= numSteps; ++step)
    {
        int x = round(xf);
        int y = round(yf);
        if (x >= 0 && x < target->width && y >= 0 && y < target->height){
            target->buffer[y * target->stride + x] = color;
        }
        xf += xIncrement;
        yf += yIncrement;
    }
}

static void drawShadedLine(int x0, int y0, int z0, int x1, int y1, int z1, Bitmap * target, EFI_GRAPHICS_OUTPUT_BLT_PIXEL (*colorfunc)(float[3]))
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int dz = z1 - z0;
    EFI_UINT32 xSteps = abs(dx);
    EFI_UINT32 ySteps = abs(dy);
    EFI_UINT32 numSteps = MAX(xSteps, ySteps);
    float numStepsF = (float) numSteps;
    float xIncrement = dx / numStepsF;
    float yIncrement = dy / numStepsF;
    float zIncrement = dz / numStepsF;

    float xf = x0;
    float yf = y0;
    float zf = z0;
    for (EFI_UINT32 step = 0; step <= numSteps; ++step)
    {
        float pos[3] = {xf,yf,zf};
        int x = round(xf);
        int y = round(yf);

        if (x >= 0 && x < target->width && y >= 0 && y < target->height){
            target->buffer[y * target->stride + x] = colorfunc(pos);
        }
        xf += xIncrement;
        yf += yIncrement;
        zf += zIncrement;
    }
}

static void bltBitmapScaled(Bitmap *target, Bitmap *source, EFI_UINTN x_start, EFI_UINTN y_start, EFI_UINTN x_end, EFI_UINTN y_end)
{
    float x_scale = ((float)(x_end - x_start)) / source->width;
    float y_scale = ((float)(y_end - y_start)) / source->height;

    for (int ty = y_start; ty < y_end; ty += 1)
    {
        for (int tx = x_start; tx < x_end; tx += 1)
        {
            int sx = (tx - x_start) / x_scale;
            int sy = (ty - y_start) / y_scale;
            size_t sidx = (sy * source->stride) + sx;
            size_t tidx = (ty * target->stride) + tx;

            *(EFI_INT32 *)&target->buffer[tidx] = *(EFI_INT32 *)&(source->buffer[sidx]);
        }
    }
}

static void bltBitmapScaledXor(Bitmap *target, Bitmap *source, EFI_UINTN x_start, EFI_UINTN y_start, EFI_UINTN x_end, EFI_UINTN y_end)
{
    float x_scale = ((float)(x_end - x_start)) / source->width;
    float y_scale = ((float)(y_end - y_start)) / source->height;

    for (int ty = y_start; ty < y_end; ty += 1)
    {
        for (int tx = x_start; tx < x_end; tx += 1)
        {
            int sx = (tx - x_start) / x_scale;
            int sy = (ty - y_start) / y_scale;
            size_t sidx = (sy * source->stride) + sx;
            size_t tidx = (ty * target->stride) + tx;

            *(EFI_INT32 *)&target->buffer[tidx] ^= *(EFI_INT32 *)&(source->buffer[sidx]);
        }
    }
}

#endif