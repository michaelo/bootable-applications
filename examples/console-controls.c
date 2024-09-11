// TODO: Showcase reading arrow-buttons to e.g control an object on screen
#include "lil_uefi/lil_uefi.h"
#include <stddef.h>

void fill(EFI_UINT32 *frame_buffer, EFI_UINT64 frame_buffer_size, EFI_UINT32 r, EFI_UINT32 g, EFI_UINT32 b)
{
    for (EFI_UINT64 idx = 0; idx < frame_buffer_size / 4; idx += 1)
    {
        frame_buffer[idx] = 0 | (r << 16) | (g << 8) | (b);
    }
}

typedef struct object_t {
    EFI_UINT32* buf;
    EFI_UINT64 w;
    EFI_UINT64 h;
    // All below is fixed point, div by 8 to get integer value
    EFI_UINT64 vx;
    EFI_UINT64 vy;
    EFI_UINT64 x;
    EFI_UINT64 y;
} object_t;

// entry point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    EFI_BOOT_SERVICES *boot_services = system_table->BootServices;
    EFI_STATUS status;

    EFI_GUID gfx_out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out_prot;
    status = boot_services->LocateProtocol(&gfx_out_guid, 0, (void**)&gfx_out_prot);
    
    if(status != 0) {
        return status;
    }

    gfx_out_prot->SetMode(gfx_out_prot, 0);

    EFI_UINTN event;
    EFI_INPUT_KEY key;

    EFI_UINT32* frame_buffer_addr = (EFI_UINT32*)gfx_out_prot->Mode->frame_buffer_base;
    EFI_UINT64 frame_buffer_size = gfx_out_prot->Mode->frame_buffer_size;

    // Initiate a simple game-object
    EFI_UINT32 src_image[64];
    fill(src_image, 64*4, 255, 255, 255);
    object_t obj = {.buf = src_image, .h=8, .w=8, .vx=2, .vy=2, .x=8, .y=8};
    
    // Fill background
    fill(frame_buffer_addr, frame_buffer_size, 0, 0, 0);

    int alive = 1;
    
    while(alive==1) {
        system_table->BootServices->Stall(10000);

        // User event
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);
        switch(key.ScanCode) {
            case EFI_SCAN_Up:
                obj.vy -= 1;
                break;
            case EFI_SCAN_Down:
                obj.vy += 1;
                break;
            case EFI_SCAN_Left:
                obj.vx -= 1;
                break;
            case EFI_SCAN_Right:
                obj.vx += 1;
                break;
            default:
                switch(key.UnicodeChar) {
                    case 13:
                        alive = 0;
                        break;
                    default:
                        break;
                }
                break;
        }

        // clear?
        
        // Render by blt (block transfer) of pixels from source to screen buffer
        gfx_out_prot->Blt(gfx_out_prot, src_image, 0, 0, 0, obj.x/8, obj.y/8, obj.w, obj.h, 8);

        // Animate
        obj.x += obj.vx;
        obj.y += obj.vy;

        // Bounds checks
        if(obj.x < 0 && obj.vx < 0 || (obj.x/8)+obj.w > gfx_out_prot->Mode->info->HorizontalResolution) {
            obj.vx *= -1;
        }

        if(obj.y < 0 && obj.vy < 0 || (obj.y/8)+obj.w > gfx_out_prot->Mode->info->VerticalResolution) {
            obj.vy *= -1;
        }
    }

    return (0);
}
