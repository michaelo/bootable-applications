#include "lil_uefi/lil_uefi.h"
#include "shared/rand.h"
#include "shared/draw.h"
#include "shared/math.h"
#include "shared/hsv.h"
#include "shared/memory.h"
#include "shared/text.h"

#define SCRATCH_SIZE 1024

void benchmark(void (*func)(void), int num, EFI_EVENT * event, int * count, int * time)
{
    *count = 0;
    *time = 0;
    while (++(*count) < num){
        func();
        if (!efi_global_system_table->BootServices->CheckEvent(event)) (*time)++;
    }
}

EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out;
Bitmap screen;
Color_BGRA pink;

void fillScreenNaive()
{
    fillBitmap(&screen, pink);
}

void fillScreenBlt()
{
    fillScreen(gfx_out, pink);
}

void fillScreenMemset()
{
    memset(screen.buffer, 0xff, screen.stride * screen.height * 4);
}

// entry point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    efi_initialize_global_state(system_table);
    useFloatingPointMath();

    EFI_BOOT_SERVICES *boot_services = system_table->BootServices;
    EFI_STATUS status;

    system_table->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    EFI_GUID gfx_out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    boot_services->LocateProtocol(&gfx_out_guid, 0, (void **)&gfx_out);

    gfx_out->SetMode(gfx_out, 0);

    initializeBitmapFromScreenBuffer(&screen, gfx_out);    
    pink = color(255,128,0);
    unsigned short text[SCRATCH_SIZE];


    int second = 10000000;
    int ms = second / 1000;
    EFI_UINTN index;
    EFI_EVENT timerEvent;
    boot_services->CreateEvent(EFI_EVT_TIMER, EFI_TPL_CALLBACK, NULL, (void *)NULL, &timerEvent);
    boot_services->SetTimer(timerEvent, EFI_TIMER_DELAY_Periodic, ms * 10);

    EFI_UINTN keyEvent;
    EFI_INPUT_KEY key;
    for (;;)
    {
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &keyEvent);
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);

        int bltCount, naiveCount, memsetCount;
        int bltTime, naiveTime, memsetTime;

        benchmark(fillScreenBlt, 5000, timerEvent, &bltCount, &bltTime);
        benchmark(fillScreenMemset, 5000, timerEvent, &memsetCount, &memsetTime);
        benchmark(fillScreenNaive, 5000, timerEvent, &naiveCount, &naiveTime);

        FormatterZ(text, SCRATCH_SIZE, L"Blt: \t %d iterations in approx %d ms", bltCount, bltTime * 10);
        renderStringOutline(&screen, 50, 100, color(0,0,0), color(255,255,255), 1, 8, text);
        
        FormatterZ(text, SCRATCH_SIZE, L"Memset: \t %d iterations in approx %d ms", memsetCount, memsetTime * 10);
        renderStringOutline(&screen, 50, 200, color(0,0,0), color(255,255,255), 1, 8, text);

        FormatterZ(text, SCRATCH_SIZE, L"Naive: \t %d iterations in approx %d ms", naiveCount, naiveTime * 10);
        renderStringOutline(&screen, 50, 150, color(0,0,0), color(255,255,255), 1, 8, text);
    }

    return 0;
}
