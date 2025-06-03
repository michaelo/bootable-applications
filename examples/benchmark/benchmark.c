#include "lil_uefi/lil_uefi.h"
#include "shared/rand.h"
#include "shared/draw.h"
#include "shared/math.h"
#include "shared/hsv.h"
#include "shared/memory.h"
#include "shared/text.h"
#include "shared/screen_selectres.h"

#define SCRATCH_SIZE 1024
#define NAME_SIZE 60

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
        .timeSeconds = 1,
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
Color_BGRA green;
Color_BGRA blue;
Color_BGRA red;

Bitmap * backBuffer;
unsigned short text[SCRATCH_SIZE];

int white = 0xff;

void runAllBenchmarks(Benchmark * benchmarks, int numBenchmarks)
{
        for(int i = 0; i < numBenchmarks; i++)
        {
            runBenchmark(&benchmarks[i]);            
        }

        for(int i = 0; i < numBenchmarks; i++)
        {
            FormatterZ(text, SCRATCH_SIZE, L"%s: %d iterations in approx %d ms", benchmarks[i].name, benchmarks[i].num, (int)(benchmarks[i].timeSeconds * 1000));
            renderStringOutline(&screen, 50, 50 + (i * 16), color(0,0,0), color(255,255,255), 1, 8, text);
        }
}


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

void fillBitmapMemsetUnrolled(void * state)
{
    memset_unrolled(backBuffer->buffer, *(int*)state, screen.stride * screen.height);
}

void fillBitmapMemsetVector(void * state)
{
    memset_vector(backBuffer->buffer, *(int*)state, screen.stride * screen.height);
}

void fillBitmapMemsetVectorArray(void * state)
{
    memset_vector_array(backBuffer->buffer, (unsigned int*)state, screen.stride * screen.height);
}


void calcSin(void * state)
{
    sin(0.5f); // Just a dummy operation to test performance
}

void calcCos(void * state)
{
    cos(0.5f); // Just a dummy operation to test performance
}

void calcTan(void * state)
{
    tan(0.5f); // Just a dummy operation to test performance
}

void calcSqrt(void * state)
{
    sqrt(0.5f); // Just a dummy operation to test performance
}

// entry point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    efi_initialize_global_state(system_table);
    useFloatingPointMath();

    // Prevent automatic 5min abort
    system_table->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    gfx_out = GetModeGraphics(system_table);
    SelectResolution(system_table);
    
    initializeBitmapFromScreenBuffer(&screen, gfx_out);    
    backBuffer = allocateBitmap(screen.stride, screen.height);
    backBuffer->width = screen.width;

    orange = color(255,128,0);
    pink = color(255,200,200);
    green = color(0,255,0);
    blue = color(30,30,255);
    red = color(255,0,0);

    EFI_UINTN keyEvent;
    EFI_INPUT_KEY key;
    
    Color_BGRA colors[] = {
        red,
        blue,
        orange,
        green,
    };

    Benchmark benchmarks[] = {
        createBenchmark(L"FillScreenNaive", fillScreenNaive, &pink),
        createBenchmark(L"FillScreenMemSet", fillScreenMemset, &white),
        createBenchmark(L"FillScreenBlt", fillScreenBlt, &orange),
        createBenchmark(L"FillBitmapNaive", fillBitmapNaive, &green),
        createBenchmark(L"FillBitmapMemSet", fillBitmapMemset, &white),
        createBenchmark(L"FillBitmapMemSetUnrolled", fillBitmapMemsetUnrolled, &blue),
        createBenchmark(L"FillBitmapMemSetVector", fillBitmapMemsetVector, &red),
        createBenchmark(L"FillBitmapMemSetVectorArray", fillBitmapMemsetVectorArray, colors),
        createBenchmark(L"BitmapScreenMemcpy", bitmapScreenMemcpy, 0),
        createBenchmark(L"BitmapScreenBlt", bitmapScreenBlt, 0),
        createBenchmark(L"CalcSin", calcSin, 0),
        createBenchmark(L"CalcCos", calcCos, 0),
        createBenchmark(L"CalcTan", calcTan, 0),
        createBenchmark(L"CalcSqrt", calcSqrt, 0)
    };

    int numBenchmarks = sizeof(benchmarks) / sizeof(Benchmark);

    runAllBenchmarks(benchmarks, numBenchmarks);
    for (;;)
    {
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &keyEvent);
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);
        runAllBenchmarks(benchmarks, numBenchmarks);
    }

    return 0;
}
