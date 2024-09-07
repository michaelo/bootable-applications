// Lil' UEFI Example
#include "lil_uefi/lil_uefi.h"
#include <stddef.h>
// #include <wchar.h>
// #include "../uefi-simple/"

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

// EfiMain

EFI_UINTN
EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    EFI_BOOT_SERVICES *boot_services = system_table->BootServices;

    EFI_GUID gfx_out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gfx_out_prot = 0;
    EFI_STATUS gfx_out_prot_locate_status = boot_services->LocateProtocol(&gfx_out_guid, 0, &gfx_out_prot);

    gfx_out_prot->SetMode(gfx_out_prot, 0);

    EFI_EVENT event;
    EFI_INPUT_KEY key;

    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* out = system_table->ConOut;

    EFI_TIME time;

    system_table->RuntimeServices->GetTime(&time, NULL);

    out->ClearScreen(out);
    out->EnableCursor(out, 1);

    // swprintf("test");

    out->SetCursorPosition(out, 0, 0);
    out->TestString(out, L"Hello,");
    out->OutputString(out, L"Hello,");
    out->SetCursorPosition(out, 0, 1);
    out->OutputString(out, L"NDC!");

    out->SetCursorPosition(out, 40, 40);
    out->OutputString(out, L"AAAAAAAAAAAAAAAAAAAAAA!");

    for(;;) {
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &event);
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);
        if(key.UnicodeChar == 13) break;
    }

    return (0);
}
