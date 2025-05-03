#include "lil_uefi/lil_uefi.h"
#include "memory.h"

EFI_GRAPHICS_OUTPUT_PROTOCOL * graphics;
EFI_UINT32 width;
EFI_UINT32 height;
EFI_UINT32 stride;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL * pixels;
EFI_UINT32 bufferSize;

void initialize_drawing(EFI_GRAPHICS_OUTPUT_PROTOCOL * graphics_in)
{
    graphics = graphics_in;
    pixels = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *) graphics->Mode->frame_buffer_base;    
    width =  graphics->Mode->info->HorizontalResolution;
    height = graphics->Mode->info->VerticalResolution;
    stride = graphics->Mode->info->PixelsPerScanLine;
    bufferSize = graphics->Mode->frame_buffer_size;
}

EFI_GRAPHICS_OUTPUT_BLT_PIXEL randomColor()
{
    EFI_UINT32 color = rand() & 0xffffff;
    return *((EFI_GRAPHICS_OUTPUT_BLT_PIXEL*)&color);
}

void fillRandom(EFI_GRAPHICS_OUTPUT_BLT_PIXEL * buffer, EFI_UINT32 size)
{
    for (EFI_UINT64 idx = 0; idx < size; idx += 1)
    {
        buffer[idx] = randomColor();
    }
}

void fill(EFI_GRAPHICS_OUTPUT_BLT_PIXEL * buffer, EFI_UINT32 size, EFI_GRAPHICS_OUTPUT_BLT_PIXEL color)
{
    for (EFI_UINT64 idx = 0; idx < size; idx += 1)
    {
        buffer[idx] = color;
    }
}

EFI_GRAPHICS_OUTPUT_BLT_PIXEL color(EFI_UINT32 r, EFI_UINT32 g, EFI_UINT32 b)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL p;
    p.Red = r;
    p.Green = g;
    p.Blue = b;
    return p;
}

void drawRectangle(EFI_UINT32 dx, EFI_UINT32 dy, EFI_UINT32 width, EFI_UINT32 height, EFI_GRAPHICS_OUTPUT_BLT_PIXEL color)
{
    EFI_UINT32 * buffer = (EFI_UINT32 *) &color;
    graphics->Blt(graphics, buffer, EFI_GRAPHICS_OUTPUT_BLT_OPERATION_VideoFill, 0, 0, dx, dy, width, height, 0);
}

void clear(EFI_GRAPHICS_OUTPUT_BLT_PIXEL color)
{
    drawRectangle(0, 0, width, height, color);
}

void clearRandom()
{
    fillRandom(pixels, bufferSize / sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
}

typedef struct {
    EFI_UINT32 width;
    EFI_UINT32 height;
    EFI_UINT32 strideBytes;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL * buffer;
} SPRITE;

SPRITE * createSprite(EFI_UINT32 width, EFI_UINT32 height)
{
    SPRITE * sprite = (SPRITE*) uefi_malloc(sizeof(SPRITE) + sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * width * height);
    sprite->width = width;
    sprite->height = height;
    sprite->strideBytes = width * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
    sprite->buffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *) (&(sprite->buffer) + sizeof(EFI_UINT32));
    return sprite;
}

SPRITE * loadSprite(EFI_UINT32 width, EFI_UINT32 height, const unsigned int * buffer)
{
    SPRITE * bitmap = uefi_malloc(sizeof(SPRITE));
    bitmap->width = width;
    bitmap->height = height;
    bitmap->strideBytes = width * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
    bitmap->buffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL*) buffer;
    return bitmap;
}

void destroySprite(SPRITE * sprite)
{
    uefi_free(sprite);
}

void drawSprite(EFI_UINT32 dx, EFI_UINT32 dy, SPRITE * sprite)
{
    graphics->Blt(graphics, (EFI_UINT32*)sprite->buffer, EFI_GRAPHICS_OUTPUT_BLT_OPERATION_BufferToVideo, 0, 0, dx, dy, sprite->width, sprite->height, sprite->strideBytes);
}

void grabScreenToSprite(EFI_UINT32 sx, EFI_UINT32 sy, SPRITE * sprite)
{
    graphics->Blt(graphics, (EFI_UINT32*)sprite->buffer, EFI_GRAPHICS_OUTPUT_BLT_OPERATION_VideoToBuffer, sx, sy, 0, 0, sprite->width, sprite->height, sprite->strideBytes);
}

void drawSpriteTransparent(EFI_UINT32 dx, EFI_UINT32 dy, SPRITE * sprite)
{
    for (EFI_UINT32 y = 0; y < sprite->height; y++)
    {
        int py = y + dy;
        for (EFI_UINT32 x = 0; x < sprite->width; x++)
        {
            int px = x + dx;
            EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixel = sprite->buffer[y * sprite->width + x];
            if (pixel.Reserved != 0) {
                pixels[py * stride + px] = pixel;
            }
        }
    }
}

