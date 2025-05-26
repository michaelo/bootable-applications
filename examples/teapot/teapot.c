#include "lil_uefi/lil_uefi.h"
#include "shared/bitmap.h"
#include "shared/draw.h"
#include "shared/lineset.h"
#include "models/teapot.h"

#define NULL 0

EFI_GRAPHICS_OUTPUT_BLT_PIXEL colorfunc(float point[3])
{
    // z values from approx -50 to 250 seems to be the range for the scaled and rotated teapot
    int zscaled = 15 + CLAMP(point[2]+10, 0, 240);
    return color(zscaled,zscaled,zscaled);
}

Lineset * loadTeapotData()
{
    Lineset * teapot_data = allocateLineset(teapotVertsCount, teapotFaceIndexesCount, 3, 1);
    memcpy(teapot_data->vertices, teapotVerts, teapotVertsCount * sizeof(float) * 3);
    memcpy(teapot_data->lines, teapotFaceIndexes, teapotFaceIndexesCount * sizeof(int) * 3);
    return teapot_data;
}

void teapot(Bitmap * backBuffer, Lineset * teapot_data, float zdeg)
{
    float modelView[4][4];
    make_identity(modelView);
    float scaleFactor = backBuffer->width / 2;
    scale(modelView, scaleFactor, scaleFactor, scaleFactor);//scale it up so it more or less fills the viewport space
    rotateY(modelView, degToRad(zdeg));//rotate around the scene y axis
    rotateX(modelView, degToRad(-135));//rotate along the scene x axis
    translate(modelView, backBuffer->width / 2, backBuffer->height * 0.6, 0);//move the object to be approx centered on the viewport
    
    //clear backbuffer:
    fillBitmap(backBuffer, color(0,0,0));

    // //render lines to backbuffer:
    renderLineset(teapot_data, modelView, backBuffer, colorfunc);
}

// static int __chkstk = 0;
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    efi_initialize_global_state(system_table);
    useFloatingPointMath();

    EFI_BOOT_SERVICES *boot_services = system_table->BootServices;
    EFI_STATUS status;


    EFI_GUID gfx_out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out_prot;
    status = boot_services->LocateProtocol(&gfx_out_guid, 0, (void **)&gfx_out_prot);
    if (status != 0)
    {
        return status;
    }
    gfx_out_prot->SetMode(gfx_out_prot, 0);

    Bitmap screen;
    initializeBitmapFromScreenBuffer(&screen, gfx_out_prot);
    Bitmap * backBuffer = allocateBitmap(screen.width, screen.height);
    Lineset * teapot_data = loadTeapotData();

    double t = 0;

    EFI_UINTN index;
    EFI_EVENT loopEvent;
    int second = 10000000;
    int fps = 60;
    status = boot_services->CreateEvent(EFI_EVT_TIMER, EFI_TPL_CALLBACK, NULL, (void *)NULL, &loopEvent);
    status = boot_services->SetTimer(loopEvent, EFI_TIMER_DELAY_Periodic, second/fps);
    for (;;)
    {
        status = boot_services->WaitForEvent(1, &loopEvent, &index);
        teapot(backBuffer, teapot_data, t);
        drawBitmapToScreen(gfx_out_prot, 0, 0, backBuffer);

        t += 0.2f;
    }

    return 0;
}
