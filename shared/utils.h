#ifndef IMPL_UTILS
#define IMPL_UTILS

#include "lil_uefi/lil_uefi.h"


#include "shared/text.h"
#include "shared/efi_status.h"

typedef enum
{
    Text,
    Graphics,
} Mode;

static void ConsoleWrite(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out, int col, int row, EFI_UINT16 *text)
{
    out->SetCursorPosition(out, col, row);
    // https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html?highlight=OutputString#efi-simple-text-output-protocol
    out->OutputString(out, text);
}

static EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *GetModeText(EFI_SYSTEM_TABLE *system_table)
{
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out_prot;

    EFI_GUID out_guid = EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID;

    if (system_table->BootServices->LocateProtocol(&out_guid, 0, (void **)&out_prot) != 0)
    {
        return 0;
    }

    return out_prot;
}

static EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *SetModeText(EFI_SYSTEM_TABLE *system_table, EFI_UINT32 num)
{
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out_prot = GetModeText(system_table);

    if (out_prot == 0)
        return 0;
    if (out_prot->SetMode(out_prot, num) != 0)
        return 0;

    return out_prot;
}

static EFI_GRAPHICS_OUTPUT_PROTOCOL *GetModeGraphics(EFI_SYSTEM_TABLE *system_table)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *out_prot;

    EFI_GUID out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    if (system_table->BootServices->LocateProtocol(&out_guid, 0, (void **)&out_prot) != 0)
    {
        return 0;
    }

    return out_prot;
}

static EFI_GRAPHICS_OUTPUT_PROTOCOL *SetModeGraphics(EFI_SYSTEM_TABLE *system_table, EFI_UINT32 num)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *out_prot = GetModeGraphics(system_table);

    if (out_prot == 0)
        return 0;
    if (out_prot->SetMode(out_prot, num) != 0)
        return 0;

    return out_prot;
}

static EFI_GRAPHICS_OUTPUT_PROTOCOL *GetModeGraphicsAdv(EFI_SYSTEM_TABLE *system_table, EFI_HANDLE handle)
{
    EFI_CHAR16 scratch[64];
    EFI_GRAPHICS_OUTPUT_PROTOCOL *out_prot;

    EFI_GUID out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_STATUS status = system_table->BootServices->OpenProtocol(
        handle,
        &out_guid,
        (void **)&out_prot,
        NULL,
        NULL,
        EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if (status != 0)
    {
        // FormatterZ(scratch, 64, L"error: %X - %s", status, efi_error_str(status));
        // ConsoleWrite(system_table->ConOut, 0, 0, scratch);

        return NULL;
    }

    return out_prot;
}

static EFI_GRAPHICS_OUTPUT_PROTOCOL *SetModeGraphicsAdv(EFI_SYSTEM_TABLE *system_table, EFI_HANDLE handle, EFI_UINT32 num)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *out_prot = GetModeGraphicsAdv(system_table, handle);

    if (out_prot == 0)
        return 0;
    if (out_prot->SetMode(out_prot, num) != 0)
        return 0;

    return out_prot;
}

static void BlockForKey(EFI_SYSTEM_TABLE *system_table, EFI_UINT16 keychar)
{
    EFI_UINTN event;
    EFI_INPUT_KEY key;

    for (;;)
    {
        // https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html?highlight=waitforevent#id7
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &event);

        // https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html?highlight=waitforkey#efi-simple-text-input-protocol-readkeystroke
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);

        if (key.UnicodeChar == keychar)
            break;
    }
}

#endif