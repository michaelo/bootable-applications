#include "lil_uefi/lil_uefi.h"
#include <stddef.h>
#include "shared/utils.h"
#include "shared/draw.h"
#include "shared/screen_selectres.h"
#include "shared/text.h"

EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    // Prevent automatic 5min abort
    system_table->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
    initialize_memory(system_table->BootServices);
    *((int volatile *)&_fltused)=0; //prevent LTO from removing the marker symbol _fltused

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx = GetModeGraphics(system_table);
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = system_table->ConOut;
    out->ClearScreen(out);

    SelectResolution(system_table);
    fillScreen(gfx, color(96,0,0));

    drawRectangleToScreen(gfx, 10, 10, 95, 95, color(128, 0, 0));

    drawRectangleToScreen(gfx, 150, 70, 95, 120, color(128, 128, 0));

    Bitmap * screen = bitmapFromScreenBuffer(gfx);
    drawLineToBitmap(0,0,120,120,screen, color(255,255,0));
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg = color(0, 0, 0);
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg = color(255, 255, 255);


    renderStringFg(screen, 100, 100, fg, L"Woop woop - this is all good! ha æøå da");

    renderStringFg(screen, 100, 200, fg, L"Press Enter to shutdown");

    BlockForKey(system_table, 13);
    destroyBitmap(screen);
    system_table->RuntimeServices->ResetSystem(EFI_RESET_TYPE_Shutdown, 0, 0, NULL);

    return (0);
}
