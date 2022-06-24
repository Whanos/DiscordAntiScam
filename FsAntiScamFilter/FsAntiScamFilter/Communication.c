#include "DriverHeaders.h"

NTSTATUS FsAntiScamMessageReceived(
    PVOID PortCookie,
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PVOID OutputBuffer,
    ULONG OutputBufferLength,
    PULONG ReturnBufferLength
) {
    PCHAR ApplicationMessage = (PCHAR)InputBuffer;
    return STATUS_SUCCESS;
}
