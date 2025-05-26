// https://uefi.org/specs/UEFI/2.10/Apx_D_Status_Codes.html
#include "lil_uefi/lil_uefi.h"

EFI_CHAR16 *efi_error_str(EFI_STATUS status_lsb)
{
    // Table D.3 EFI_STATUS Error Codes (High Bit Set)
    switch (status_lsb)
    {
    case 1:
        return (EFI_CHAR16 *)L"The image failed to load.";

    case 2:
        return (EFI_CHAR16 *)L"A parameter was incorrect.";

    case 3:
        return (EFI_CHAR16 *)L"The operation is not supported.";

    case 4:
        return (EFI_CHAR16 *)L"The buffer was not the proper size for the request.";

    case 5:
        return (EFI_CHAR16 *)L"The buffer is not large enough to hold the requested data. The required buffer size is returned in the appropriate parameter when this error occurs.";

    case 6:
        return (EFI_CHAR16 *)L"There is no data pending upon return.";

    case 7:
        return (EFI_CHAR16 *)L"The physical device reported an error while attempting the operation.";

    case 8:
        return (EFI_CHAR16 *)L"The device cannot be written to.";

    case 9:
        return (EFI_CHAR16 *)L"A resource has run out.";

    case 10:
        return (EFI_CHAR16 *)L"An inconstancy was detected on the file system causing the operating to fail.";

    case 11:
        return (EFI_CHAR16 *)L"There is no more space on the file system.";

    case 12:
        return (EFI_CHAR16 *)L"The device does not contain any medium to perform the operation.";

    case 13:
        return (EFI_CHAR16 *)L"The medium in the device has changed since the last access.";

    case 14:
        return (EFI_CHAR16 *)L"The item was not found.";

    case 15:
        return (EFI_CHAR16 *)L"Access was denied.";

    case 16:
        return (EFI_CHAR16 *)L"The server was not found or did not respond to the request.";

    case 17:
        return (EFI_CHAR16 *)L"A mapping to a device does not exist.";

    case 18:
        return (EFI_CHAR16 *)L"The timeout time expired.";

    case 19:
        return (EFI_CHAR16 *)L"The protocol has not been started.";

    case 20:
        return (EFI_CHAR16 *)L"The protocol has already been started.";

    case 21:
        return (EFI_CHAR16 *)L"The operation was aborted.";

    case 22:
        return (EFI_CHAR16 *)L"An ICMP error occurred during the network operation.";

    case 23:
        return (EFI_CHAR16 *)L"A TFTP error occurred during the network operation.";

    case 24:
        return (EFI_CHAR16 *)L"A protocol error occurred during the network operation.";

    case 25:
        return (EFI_CHAR16 *)L"The function encountered an internal version that was incompatible with a version requested by the caller.";

    case 26:
        return (EFI_CHAR16 *)L"The function was not performed due to a security violation.";

    case 27:
        return (EFI_CHAR16 *)L"A CRC error was detected.";

    case 28:
        return (EFI_CHAR16 *)L"Beginning or end of media was reached";

    case 31:
        return (EFI_CHAR16 *)L"The end of the file was reached.";

    case 32:
        return (EFI_CHAR16 *)L"The language specified was invalid.";

    case 33:
        return (EFI_CHAR16 *)L"The security status of the data is unknown or compromised and the data must be updated or replaced to restore a valid security status.";

    case 34:
        return (EFI_CHAR16 *)L"There is an address conflict address allocation";

    case 35:
        return (EFI_CHAR16 *)L"A HTTP error occurred during the network operation.";
    default:
        return (EFI_CHAR16 *)L"Unknown error";
    }
}

EFI_CHAR16 *efi_warning_str(EFI_STATUS status_lsb)
{
    // Table D.4 EFI_STATUS Warning Codes (High Bit Clear)
    switch (status_lsb)
    {
    case 1:
        return (EFI_CHAR16 *)L"The string contained one or more characters that the device could not render and were skipped.";

    case 2:
        return (EFI_CHAR16 *)L"The handle was closed, but the file was not deleted.";

    case 3:
        return (EFI_CHAR16 *)L"The handle was closed, but the data to the file was not flushed properly.";

    case 4:
        return (EFI_CHAR16 *)L"The resulting buffer was too small, and the data was truncated to the buffer size.";

    case 5:
        return (EFI_CHAR16 *)L"The data has not been updated within the timeframe set by local policy for this type of data.";

    case 6:
        return (EFI_CHAR16 *)L"The resulting buffer contains UEFI-compliant file system.";

    case 7:
        return (EFI_CHAR16 *)L"The operation will be processed across a system reset.";
    default:
        return (EFI_CHAR16 *)L"Unknown warning";
    }
}

EFI_CHAR16 *efi_status_str(EFI_STATUS status)
{
    if (status == 0)
    {
        return (EFI_CHAR16 *)L"The operation completed successfully.";
    }

    // TODO: Support 32 bit?
    // TODO: Support the remainding variants (OEM vs reserverd EFI)
    if ((status & (1ULL<<63)) != 0)
    {
        return efi_error_str(status & 0x7FFFFFFFFFFFFFFF);
    }
    else
    {
        return efi_warning_str(status & 0x7FFFFFFFFFFFFFFF);
    }
    return 0;
}
