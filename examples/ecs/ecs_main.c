#include "lil_uefi/lil_uefi.h"
#include "shared/rand.h"
#include "shared/draw.h"
#include "shared/math.h"
#include "shared/memory.h"
#include "shared/text.h"
#include "component.h"
#include "entity.h"

// entry point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    efi_initialize_global_state(system_table);
    useFloatingPointMath();

    Entity particle = createEntity(); // Example entity
    addComponentToEntity(&particle, COMPONENT_TYPE_POSITION); 
    addComponentToEntity(&particle, COMPONENT_TYPE_VELOCITY);

    Entity model = createEntity(); // Another example entity
    addComponentToEntity(&model, COMPONENT_TYPE_POSITION);
    addComponentToEntity(&model, COMPONENT_TYPE_ROTATION);
    addComponentToEntity(&model, COMPONENT_TYPE_SCALE);

    EFI_BOOT_SERVICES *boot_services = system_table->BootServices;
    EFI_STATUS status;

    system_table->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    EFI_GUID gfx_out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out;
    boot_services->LocateProtocol(&gfx_out_guid, 0, (void **)&gfx_out);

    gfx_out->SetMode(gfx_out, 0);

    Bitmap screen;
    initializeBitmapFromScreenBuffer(&screen, gfx_out);    
    Bitmap * plasmaBuffer = allocateBitmap(240, 180);
    Bitmap * backBuffer = allocateBitmap(screen.width, screen.height);

    Bitmap * cache[128];
    for (int i = 0; i < 128; i++) cache[i] = 0;

    float t = 0;

    float speed = 0.07f;
    int fps = 60;
    EFI_UINTN index;
    EFI_EVENT loopEvent;
    boot_services->CreateEvent(EFI_EVT_TIMER, EFI_TPL_CALLBACK, NULL, (void *)NULL, &loopEvent);
    boot_services->SetTimer(loopEvent, EFI_TIMER_DELAY_Periodic, 10000000 / fps);
    for (;;)
    {
        status = boot_services->WaitForEvent(1, &loopEvent, &index);

        // Scale lowres bitmap to screen-sized backbuffer
        bltBitmapScaled(backBuffer, plasmaBuffer, 0, 0, backBuffer->width, backBuffer->height);
        // Copy from backbuffer to screen
        drawBitmapToScreen(gfx_out, 0, 0, backBuffer);

        t += speed;
    }

    return 0;
}
