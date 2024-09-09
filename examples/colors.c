#include "lil_uefi/lil_uefi.h"
#include <stddef.h>

void* memset(void *ptr, int x, size_t n)
{
}

void fill(EFI_UINT32 *frame_buffer, EFI_UINT64 frame_buffer_size, EFI_UINT32 r, EFI_UINT32 g, EFI_UINT32 b)
{
    for (EFI_UINT64 idx = 0; idx < frame_buffer_size / 4; idx += 1)
    {
        frame_buffer[idx] = 0 | (r << 16) | (g << 8) | (b);
    }
}

// entry point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    EFI_BOOT_SERVICES *boot_services = system_table->BootServices;

    EFI_GUID gfx_out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out_prot = 0;
    EFI_STATUS gfx_out_prot_locate_status = boot_services->LocateProtocol(&gfx_out_guid, 0, &gfx_out_prot);

    gfx_out_prot->SetMode(gfx_out_prot, 0);

    EFI_EVENT event;
    EFI_INPUT_KEY key;

    EFI_UINT32* frame_buffer_addr = (EFI_UINT32*)gfx_out_prot->Mode->frame_buffer_base;
    EFI_UINT64 frame_buffer_size = gfx_out_prot->Mode->frame_buffer_size;
    
    fill(frame_buffer_addr, frame_buffer_size, 240, 127, 34);

    EFI_TIME time;
    for(;;) {
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &event);
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);
        if(key.UnicodeChar == 13) break;

        system_table->RuntimeServices->GetTime(&time, NULL);
        fill(frame_buffer_addr, frame_buffer_size, (time.Second*50)%256, (time.Second*33)%256, (time.Second*7)%256);
    }

    return (0);
}
