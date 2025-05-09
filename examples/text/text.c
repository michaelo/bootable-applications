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

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx = GetModeGraphics(system_table);
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = system_table->ConOut;
    out->ClearScreen(out);

    SelectResolution(system_table);
    fill(gfx, color(96,0,0));

    drawRectangle(gfx, 10, 10, 95, 95, color(128, 0, 0));

    drawRectangle(gfx, 150, 70, 95, 120, color(128, 128, 0));

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg = color(0, 0, 0);
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg = color(255, 255, 255);

    // renderChar(gfx, 100, 100, bg, fg, 'A');
    // renderString(gfx, 100, 100, bg, fg, L"Woop woop - this is all good! ha æøå da");
    renderStringFg(gfx, 100, 100, fg, L"Woop woop - this is all good! ha æøå da");

    renderStringFg(gfx, 100, 200, fg, L"Press Enter to shutdown");

    BlockForKey(system_table, 13);
    system_table->RuntimeServices->ResetSystem(EFI_RESET_TYPE_Shutdown, 0, 0, NULL);

    return (0);
}
