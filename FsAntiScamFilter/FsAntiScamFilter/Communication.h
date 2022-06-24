#pragma once

#ifndef NTSTATUS
#include "DriverHeaders.h"
#endif

extern NTSTATUS FsAntiScamMessageReceived(
    PVOID PortCookie,
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PVOID OutputBuffer,
    ULONG OutputBufferLength,
    PULONG ReturnBufferLength
);
