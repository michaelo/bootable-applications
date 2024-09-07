// Lil' UEFI Example
#include "lil_uefi/lil_uefi.h"
#include <stddef.h>

#define ArrayCount(a) ((sizeof(a)) / sizeof((a)[0]))

void memset(void *ptr, int x, size_t n)
{
}

void fill(EFI_UINT32 *frame_buffer, EFI_UINT64 frame_buffer_size, EFI_UINT32 r, EFI_UINT32 g, EFI_UINT32 b)
{
    for (EFI_UINT64 idx = 0; idx < frame_buffer_size / 4; idx += 1)
    {
        frame_buffer[idx] = 0 | (r << 16) | (g << 8) | (b);
    }
}

EFI_UINTN
EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    // rjf: grab boot services
    EFI_BOOT_SERVICES *boot_services = system_table->BootServices;

    // rjf: grab graphics output protocol
    EFI_GUID gfx_out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out_prot = 0;
    EFI_STATUS gfx_out_prot_locate_status = boot_services->LocateProtocol(&gfx_out_guid, 0, &gfx_out_prot);

    // rjf: set gfx out protocol mode to zero
    gfx_out_prot->SetMode(gfx_out_prot, 0);

    EFI_EVENT event;
    system_table->ConOut->SetCursorPosition(system_table->ConOut, 0, 0);
    system_table->ConOut->OutputString(system_table->ConOut, L"Hello,");
    system_table->ConOut->SetCursorPosition(system_table->ConOut, 0, 1);
    system_table->ConOut->OutputString(system_table->ConOut, L"NDC!");
    system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &event);

    return (0);
}
