#include "lil_uefi/lil_uefi.h"
#include "shared/rand.h"
#include "shared/drawing.h"
#include "shared/math.h"
#include "shared/colorspace.h"

#define NULL 0

// Originally inspired by https://lodev.org/cgtutor/plasma.html
#define dist(a, b, c, d) sqrt((double)((a - c) * (a - c) + (b - d) * (b - d)))

double * InitializeStaticData()
{
    double * data = uefi_malloc(stride * height * sizeof(double));
    double p1x = width * 0.25;
    double p2x = width * 0.75;
    double p1y = height * 0.25;
    double p2y = height * 0.375;
    double scaleFactor = (width / 20);

    for (int y = 0; y < height; y += 1)
    {
        for (int x = 0; x < width; x += 1)
        {
            data[y * stride + x] = 
                sin(dist(x, y, p1x,  p1y) / scaleFactor) + // constant for each x,y. Distance from a point at 0.25,0.25 in screen coords
                sin(dist(x, y, p2x,  p2y) / scaleFactor) + // constant for each x,y. Distance from a point at 0.75,0.375 in screen coords
                0;
        }
    }
    return data;
}

double * staticData = 0;
SPRITE * backBuffer = 0;

int interlacing = 0;
void plasma(double time)
{
    if (staticData == 0) staticData = InitializeStaticData();
    if (backBuffer == 0) {
        fill(backBuffer->buffer, backBuffer->width * backBuffer->height, color(0,0,0));
        backBuffer = createSprite(width, height);
    }
    interlacing = (interlacing + 1) % 2;

    Color_HSVA hsva;
    hsva.s = 255;
    hsva.v = 255;
    hsva.a = 255;

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixel;

    int w = width;
    int h = height;
    double xPos = w / 2 + (w * cos(time / 13));
    double yPos = h / 3 + (h * cos(time / 17));

    double p3x = w * 0.75;
    double p4y = h * 0.5;

    double scaleFactor = (w / 20);

    for (int y = interlacing; y < h; y += 2)
    {
        for (int x = 0; x < w; x += 1)
        {
            int pos = y * stride + x;        
            double value = 0.0 +
                           staticData[pos] + 
                           sin(dist(x, y, p3x, yPos) / scaleFactor) + // moving vertically. Distance from a moving point starting at 0.75,0.25 in screen coords
                           sin(dist(x, y, xPos, p4y) / scaleFactor) + // moving horizontally. Distance from a moving point starting at 0.5,0.5 in screen coords
                           0;

            hsva.h = (unsigned char)(value * 90) + 35;
            backBuffer->buffer[pos] = HsvToRgb(hsva);
        }
    }
    drawSprite(0,0,backBuffer);
}

// entry point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    EFI_BOOT_SERVICES *boot_services = system_table->BootServices;
    EFI_STATUS status;

    initialize_memory(boot_services);

    EFI_GUID gfx_out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out_prot;
    status = boot_services->LocateProtocol(&gfx_out_guid, 0, (void **)&gfx_out_prot);
    if (status != 0)
    {
        return status;
    }
    gfx_out_prot->SetMode(gfx_out_prot, 0);

    initialize_drawing(gfx_out_prot);

    double t = 0;
    plasma(t);

    EFI_UINTN index;
    EFI_EVENT loopEvent;
    status = boot_services->CreateEvent(EFI_EVT_TIMER, EFI_TPL_CALLBACK, NULL, (void *)NULL, &loopEvent);
    status = boot_services->SetTimer(loopEvent, EFI_TIMER_DELAY_Periodic, 166000);
    for (;;)
    {
        status = boot_services->WaitForEvent(1, &loopEvent, &index);
        plasma(t);
        t += 0.1f;
    }

    return 0;
}
