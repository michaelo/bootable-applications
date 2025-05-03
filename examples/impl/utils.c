#ifndef IMPL_UTILS
#define IMPL_UTILS

#include "lil_uefi/lil_uefi.h"

typedef enum
{
    Text,
    Graphics,
} Mode;

void ConsoleWrite(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out, int col, int row, EFI_UINT16 *text)
{
    out->SetCursorPosition(out, col, row);
    // https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html?highlight=OutputString#efi-simple-text-output-protocol
    out->OutputString(out, text);
}

EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *GetModeText(EFI_SYSTEM_TABLE *system_table)
{
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out_prot;

    EFI_GUID out_guid = EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID;

    if (system_table->BootServices->LocateProtocol(&out_guid, 0, (void **)&out_prot) != 0)
    {
        return 0;
    }

    return out_prot;
}

EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *SetModeText(EFI_SYSTEM_TABLE *system_table, EFI_UINT32 num)
{
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out_prot = GetModeText(system_table);

    if (out_prot == 0)
        return 0;
    if (out_prot->SetMode(out_prot, num) != 0)
        return 0;

    return out_prot;
}

EFI_GRAPHICS_OUTPUT_PROTOCOL *GetModeGraphics(EFI_SYSTEM_TABLE *system_table)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *out_prot;

    EFI_GUID out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    if (system_table->BootServices->LocateProtocol(&out_guid, 0, (void **)&out_prot) != 0)
    {
        return 0;
    }

    return out_prot;
}

EFI_GRAPHICS_OUTPUT_PROTOCOL *SetModeGraphics(EFI_SYSTEM_TABLE *system_table, EFI_UINT32 num)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *out_prot = GetModeGraphics(system_table);

    if (out_prot == 0)
        return 0;
    if (out_prot->SetMode(out_prot, num) != 0)
        return 0;

    return out_prot;
}

EFI_UINTN IntLen(EFI_UINTN value)
{
    if (value == 0)
        return 1;
    EFI_UINTN len = 0;
    while (value > 0)
    {
        value = value / 10;
        len++;
    }

    return len;
}

// capacity: size, excluding e.g. terminating null - must be handled outside.
// returns number of digits formatted
EFI_UINTN FormatInt(EFI_UINT16 *buffer, EFI_UINTN capacity, EFI_UINTN value)
{
    EFI_UINTN digits = IntLen(value);

    // trunc
    if (digits > capacity)
        digits = capacity;

    for (int i = digits - 1; i >= 0; i--)
    {
        EFI_UINTN digit = value % 10;
        buffer[i] = '0' + digit;
        value = value / 10;
    }

    return digits;
}

EFI_UINTN uclamp(EFI_UINTN value, EFI_UINTN low, EFI_UINTN high)
{
    if (value < low)
        value = low;
    if (value > high)
        value = high;
    return value;
}

void BlockForKey(EFI_SYSTEM_TABLE *system_table, EFI_UINT16 keychar)
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