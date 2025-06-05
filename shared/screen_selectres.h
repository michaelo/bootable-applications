#ifndef IMPL_SCREEN_SELECTRES
#define IMPL_SCREEN_SELECTRES

#include "lil_uefi/lil_uefi.h"
#include "utils.h"
#include "draw.h" // for drawRectangle
#include "color.h"
#include "text.h"

static void SelectResolution(EFI_SYSTEM_TABLE *system_table)
{
    EFI_UINTN event;
    EFI_INPUT_KEY key;

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *gfx_info;
    EFI_UINTN gfx_info_size;

    #define SCRAP_SIZE 128
    EFI_UINT16 scrap[SCRAP_SIZE];
    EFI_UINTN len = 0;

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL fg = color(255, 255, 255);
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg = colorTransparent();

    // Get all handles supporting protocol
    EFI_GUID gfxout_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_HANDLE *gfxout_handle_buf = NULL;
    EFI_UINTN num_gfxout_handles = 0;
    system_table->BootServices->LocateHandleBuffer(EFI_LOCATE_SEARCH_TYPE_ByProtocol, &gfxout_guid, NULL, &num_gfxout_handles, &gfxout_handle_buf);
    // ...

    int alive = 1;
    Bitmap screen;

    // Set initial state
    EFI_UINTN handle_idx = 0;
    EFI_UINTN mode_num = 0;
    SetModeGraphicsAdv(system_table, gfxout_handle_buf[handle_idx], mode_num);
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx;

    EFI_UINTN font_size = 8;
    while (alive)
    {
        EFI_UINTN xMargin = 20;
        EFI_UINTN y = 20;
        EFI_UINTN line_height = font_size * 1.5;

        // Read settings for current graphics mode
        gfx = GetModeGraphicsAdv(system_table, gfxout_handle_buf[handle_idx]);
        if(gfx == NULL) {
            // ERROR!
            return;
        }

        // Prepare drawing to screen buffer for current handle and mode
        initializeBitmapFromScreenBuffer(&screen, gfx);
        fillScreen(gfx, color(0,0,0));
        drawRectangleToScreen(gfx, 0, 0, 640, 480, color(128,0,128));

        // Write current resolution + help commands
        int x = xMargin;
        renderStringF(&screen, x, y, bg, fg, font_size, scrap, SCRAP_SIZE, L"--- Select resolution (ref-box is 640*480) ---");
        
        gfx->QueryMode(gfx, mode_num, &gfx_info_size, &gfx_info);

        y += line_height;
        y += line_height;
        renderStringF(&screen, xMargin, y, bg, fg, font_size, scrap, SCRAP_SIZE,
            L"Current resolution: %d x %d (handle: %d/%d, mode: %d/%d)",
            gfx_info->HorizontalResolution, gfx_info->VerticalResolution, handle_idx+1, num_gfxout_handles, mode_num+1, gfx->Mode->max_mode);

        y += line_height;
        y += line_height;
        renderString(&screen, xMargin, y, bg, fg, font_size, L"Left/Right to change mode, Up/Down to change handle. Enter when happy.");

        // Wait for response
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &event);

        // https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html?highlight=waitforkey#efi-simple-text-input-protocol-readkeystroke
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);

        switch (key.ScanCode)
        {
            // Iterative over mode
        case EFI_SCAN_Left:
            if (mode_num > 0)
                mode_num -= 1;
            break;
        case EFI_SCAN_Right:
            if (mode_num < gfx->Mode->max_mode - 1)
                mode_num += 1;
            break;
            // TODO: Iterate over protocol
        case EFI_SCAN_Down:
            if(handle_idx > 0) {
                handle_idx -= 1;
                mode_num = 0;
            }
            break;
        case EFI_SCAN_Up:
            if(handle_idx < num_gfxout_handles -1) {
                handle_idx += 1;
                mode_num = 0;
            }
            break;
        default:
            switch (key.UnicodeChar)
            {
            case 13:
                alive = 0;
                break;
            default:
                break;
            }
            break;
        };

        gfx = SetModeGraphicsAdv(system_table, gfxout_handle_buf[handle_idx], mode_num);
    }
}

#endif