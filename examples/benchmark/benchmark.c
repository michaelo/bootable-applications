#include "lil_uefi/lil_uefi.h"
#include "shared/rand.h"
#include "shared/draw.h"
#include "shared/math.h"
#include "shared/hsv.h"
#include "shared/memory.h"
#include "shared/text.h"

#define SCRATCH_SIZE 1024
#define NAME_SIZE 32

typedef struct Benchmark_s {
    short name[NAME_SIZE];
    float timeSeconds;
    int num;
    void (*target)(void *);
    void * state;
} Benchmark;

Benchmark createBenchmark(unsigned short name[], void (*target)(void *), void * state)
{
    Benchmark benchmark = {
        .timeSeconds = 3,
        .num = 0,
        .target = target,
        .state = state
    };
    memcpy(benchmark.name, name, NAME_SIZE);
    return benchmark;
}

#define EFI_ERROR(status) (status != 0)

void runBenchmark(Benchmark * benchmark)
{
    int second = 10000000;
    benchmark->num = 0;

    EFI_STATUS Status;
    EFI_EVENT TimerEvent;
    Status = efi_boot_services()->CreateEvent(EFI_EVT_TIMER, EFI_TPL_CALLBACK, NULL, (void *)NULL, &TimerEvent);
    if (EFI_ERROR (Status)) {
        benchmark->num = Status;
        return;
    }

    Status = efi_boot_services()->SetTimer(TimerEvent, EFI_TIMER_DELAY_Relative, (int)(second * benchmark->timeSeconds));
    if (EFI_ERROR (Status)) {
        efi_boot_services()->CloseEvent (TimerEvent);
        benchmark->num = Status;
        return;
    }

    do {
        Status = efi_boot_services()->CheckEvent (TimerEvent);
        benchmark->target(benchmark->state);
        benchmark->num++;
    } while (EFI_ERROR (Status));

    efi_boot_services()->CloseEvent(TimerEvent);
}

EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out;
Bitmap screen;
Color_BGRA orange;
Color_BGRA pink;
Bitmap * backBuffer;

int white = 0xff;

void fillScreenNaive(void * state)
{
    fillBitmap(&screen, *(Color_BGRA*)state);
}

void fillScreenBlt(void * state)
{
    fillScreen(gfx_out, *(Color_BGRA*)state);
}

void fillScreenMemset(void * state)
{
    memset(screen.buffer, *(int*)state, screen.stride * screen.height * 4);
}

void bitmapScreenBlt(void * state)
{
    drawBitmapToScreen(gfx_out, 0, 0, backBuffer);
}

void bitmapScreenMemcpy(void * state)
{
    memcpy(screen.buffer, backBuffer->buffer, backBuffer->stride * backBuffer->height * 4);
}

void fillBitmapNaive(void * state)
{
    fillBitmap(backBuffer, *(Color_BGRA*)state);
}

void fillBitmapMemset(void * state)
{
    memset(backBuffer->buffer, *(int*)state, screen.stride * screen.height * 4);
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
    backBuffer = allocateBitmap(screen.width, screen.height);

    orange = color(255,128,0);
    pink = color(255,200,200);
    unsigned short text[SCRATCH_SIZE];

    EFI_UINTN keyEvent;
    EFI_INPUT_KEY key;

    Benchmark benchmarks[] = {
        createBenchmark(L"FillScreenNaive", fillScreenNaive, &pink),
        createBenchmark(L"FillScreenMemSet", fillScreenMemset, &white),
        createBenchmark(L"FillScreenBlt", fillScreenBlt, &orange),
        createBenchmark(L"FillBitmapNaive", fillBitmapNaive, &pink),
        createBenchmark(L"FillBitmapMemSet", fillBitmapMemset, &white),
        createBenchmark(L"BitmapScreenBlt", bitmapScreenBlt, 0),
        createBenchmark(L"BitmapScreenMemcpy", bitmapScreenMemcpy, &white),
    };

    for (;;)
    {
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &keyEvent);
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);

        for(int i = 0; i < 7; i++)
        {
            runBenchmark(&benchmarks[i]);            
        }

        for(int i = 0; i < 7; i++)
        {
            FormatterZ(text, SCRATCH_SIZE, L"%s: %d iterations in approx %d ms", benchmarks[i].name, benchmarks[i].num, (int)(benchmarks[i].timeSeconds * 1000));
            renderStringOutline(&screen, 50, 50 + (i * 16), color(0,0,0), color(255,255,255), 1, 8, text);
        }
    }

    return 0;
}
