#include "lil_uefi/lil_uefi.h"

// Fill the provided pixel buffer with 32bit RGB-encoded colored pixels
void fill(EFI_UINT32 *frame_buffer, EFI_UINT64 frame_buffer_size, EFI_UINT32 r, EFI_UINT32 g, EFI_UINT32 b)
{
    EFI_UINT32 pixel = 0 | (r << 16) | (g << 8) | (b);

    for (EFI_UINT64 idx = 0; idx < frame_buffer_size / 4; idx += 1)
    {
        frame_buffer[idx] = pixel;
    }
}

// entry point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    EFI_STATUS status;

    // Get graphics output device
    EFI_GUID gfx_out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    struct EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out_prot;
    status = system_table->BootServices->LocateProtocol(&gfx_out_guid, 0, (void **)&gfx_out_prot);

    if (status != 0)
    {
        return status;
    }

    // Set to first available mode (resolution)
    gfx_out_prot->SetMode(gfx_out_prot, 0);
    
    // Retrieve pixel buffer start address + buffer size
    EFI_UINT32 *frame_buffer_addr = (EFI_UINT32 *)gfx_out_prot->Mode->frame_buffer_base;
    EFI_UINT64 frame_buffer_size = gfx_out_prot->Mode->frame_buffer_size;

    // Fill entire screen with single color
    fill(frame_buffer_addr, frame_buffer_size, 240, 127, 34);
    
    for (;;)
    {
        EFI_UINTN event;
        EFI_INPUT_KEY key;
        EFI_TIME time;

        // Wait for keyboard event
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &event);
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);

        // If Enter: exit. Otherwise generate new background color
        if (key.UnicodeChar == 13)
            break;

        system_table->RuntimeServices->GetTime(&time, 0);
        fill(frame_buffer_addr, frame_buffer_size, (time.Second * 50) % 256, (time.Second * 33) % 256, (time.Second * 7) % 256);
    }

    return (0);
}
