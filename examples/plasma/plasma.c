#define BASIC_TEXT

#include "lil_uefi/lil_uefi.h"
#include "shared/rand.h"
#include "shared/draw.h"
#include "shared/math.h"
#include "shared/hsv.h"
#include "shared/memory.h"
#include "shared/text.h"

#define dist(a, b, c, d) sqrt((float)((a - c) * (a - c) + (b - d) * (b - d)))

float * initializeStaticData(Bitmap * screen, Memory * memory)
{
    float * data = memory->malloc(memory, screen->width * screen->height * sizeof(float));

    //Position of the static points
    float p1x = screen->width * 0.25;
    float p2x = screen->width * 0.75;
    float p1y = screen->height * 0.25;
    float p2y = screen->height * 0.375;
    float scaleFactor = (screen->width / 20);

    for (int y = 0; y < screen->height; y += 1)
    {
        for (int x = 0; x < screen->width; x += 1)
        {
            data[y * screen->width + x] = 
                sin(dist(x, y, p1x,  p1y) / scaleFactor) + // constant for each x,y. Distance from the static point p1
                sin(dist(x, y, p2x,  p2y) / scaleFactor) + // constant for each x,y. Distance from the static point p2
                0;
        }
    }
    return data;
}

void plasma(float * staticData, Bitmap * backBuffer, float time)
{
    int w = backBuffer->width;
    int h = backBuffer->height;
    float xPos = w / 2 + (w * cos(time / 13));
    float yPos = h / 3 + (h * cos(time / 17));

    float p3x = w * 0.75;
    float p4y = h * 0.5;

    float scaleFactor = (w / 20);

    for (int y = 0; y < h; y += 1)
    {
        for (int x = 0; x < w; x += 1)
        {
            int pos = y * backBuffer->width + x;  
            float value = 
                staticData[pos]  
                + sin(dist(x, y, p3x, yPos) / scaleFactor) //Distance from point p3, which is moving vertically
                + sin(dist(x, y, xPos, p4y) / scaleFactor); //Distance from point p4, which is moving horizontally

            //value is the sum of 4 sine waves, which leaves it in the range [-4, 4]

            value += 3.5; //shift into something more visually appealing, with deep red on one end and deep purple on the other
            value = fmod(value + 8, 8) / 8; // Normalize to [0, 1]
            backBuffer->buffer[pos] = HueToRgb((unsigned char)(value * 255));
        }
    }
}

Bitmap * spriteFromChar(Bitmap * cache[128], Memory * memory, char c, int size, int outlineSize)
{
    if (!cache[c]){
        Bitmap * bitmap = allocateBitmap(size + outlineSize * 2, size + outlineSize * 2, memory);
        fillBitmap(bitmap, colorTransparent());

        Color_BGRA fg = color(0,0,0);
        Color_BGRA outline = color(255,255,255);

        float x = outlineSize;
        float y = outlineSize;
        renderCharOutline(bitmap, x, y, fg, outline, outlineSize, size, c);
        cache[c] = bitmap;
    }
    return cache[c];
}

void scrollingText(Bitmap * target, Bitmap * cache[128], Memory * memory, float t, const char * text, unsigned long long len)
{
    float speed = 40;
    float startX = target->width - fmod(t * speed, target->width * 4);

    for (int i = 0; i < len; i++)
    {
        float x = startX + i * 24;
        float y = target->height / 2 + 32 * sin(i * M_PI_M_2 / (32.0f) - t/2);
        Bitmap * bitmap = spriteFromChar(cache, memory, text[i], 24, 2);
        drawBitmapTransparent(x, y, bitmap, target);
    }
}

// entry point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    useFloatingPointMath();

    EFI_BOOT_SERVICES *boot_services = system_table->BootServices;
    EFI_STATUS status;

    system_table->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    Memory memory = initializeMemory(boot_services);

    EFI_GUID gfx_out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out;
    boot_services->LocateProtocol(&gfx_out_guid, 0, (void **)&gfx_out);

    gfx_out->SetMode(gfx_out, 0);

    Bitmap screen;
    initializeBitmapFromScreenBuffer(&screen, gfx_out);    
    Bitmap * plasmaBuffer = allocateBitmap(320, 240, &memory);
    Bitmap * backBuffer = allocateBitmap(screen.width, screen.height, &memory);
    fillBitmap(plasmaBuffer, color(0,0,0));

    Bitmap * cache[128];
    for (int i = 0; i < 128; i++) cache[i] = 0;

    float * staticData = initializeStaticData(plasmaBuffer, &memory);
    
    const char * text = "Hei NDC! H\x1fper dere klapper masse for Michael p\x1f slutten! Hilsen Terje";
    float t = 0;
    plasma(staticData, plasmaBuffer, t);
    drawBitmapToScreen(gfx_out, 0, 0, backBuffer);

    EFI_UINTN index;
    EFI_EVENT loopEvent;
    boot_services->CreateEvent(EFI_EVT_TIMER, EFI_TPL_CALLBACK, NULL, (void *)NULL, &loopEvent);
    boot_services->SetTimer(loopEvent, EFI_TIMER_DELAY_Periodic, 166000);
    for (;;)
    {
        status = boot_services->WaitForEvent(1, &loopEvent, &index);

        // Render plasma and text to low-res bitmap
        plasma(staticData, plasmaBuffer, t);
        scrollingText(plasmaBuffer, cache, &memory, t, text, 71);

        // Scale lowres bitmap to screen-sized backbuffer
        bltBitmapScaled(backBuffer, plasmaBuffer, 0, 0, backBuffer->width, backBuffer->height);

        // Copy from backbuffer to screen
        drawBitmapToScreen(gfx_out, 0, 0, backBuffer);

        t += 0.1f;
    }

    return 0;
}
