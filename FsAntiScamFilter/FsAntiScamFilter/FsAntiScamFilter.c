/*++

Module Name:

    FsAntiScamFilter.c

Abstract:

    This is the main module of the FsAntiScamFilter miniFilter driver.

Environment:

    Kernel mode

--*/

#include "DriverHeaders.h"
#include "FilterCommunication.h"
// Includes from other files
#include "Filters.h"


// Communication
PFLT_FILTER FilterHandle = NULL;
PFLT_PORT Port = NULL;
PFLT_PORT ClientPortGlobal = NULL;
// Generic functions
NTSTATUS FsFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);
// The operations we want to filter
const FLT_OPERATION_REGISTRATION FilterCallbacks[] = {
    {IRP_MJ_CREATE, 0, FsFilterPreCreate, FsFilterPostCreate},
    {IRP_MJ_READ, 0, FsFilterPreRead, FsFilterPostRead},
    {IRP_MJ_OPERATION_END}
};

// Filter details to register with
const FLT_REGISTRATION FilterRegistration = {
    sizeof(FLT_REGISTRATION),   // Size
    FLT_REGISTRATION_VERSION,   // Version
    0,                          // Flags
    NULL,                       // *ContextRegistration
    FilterCallbacks,            // *OperationRegistration
    FsFilterUnload,             // FilterUnloadCallback (Unload Function)
    NULL,                       // InstanceSetupCallback
    NULL,                       // InstanceQueryTeardownCallback
    NULL,                       // InstanceTeardownStartCallback
    NULL,                       // InstanceTeardownCompleteCallback
    NULL,                       // GenerateFileNameCallback
    NULL,                       // NormalizeNameComponentCallback
    NULL,                       // NormalizeContextCleanupCallback
    NULL,                       // TransactionNotificationCallback
};              


// Unload routine for the minifilter.
NTSTATUS FsFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {

    UNREFERENCED_PARAMETER(Flags);

    KdPrint(("Unloading FsAntiScam filter driver! \r\n"));
    FltCloseCommunicationPort(Port);
    FltUnregisterFilter(FilterHandle);

    return STATUS_SUCCESS;
}

NTSTATUS FsAntiScamConnect(
    PFLT_PORT ClientPort,
    PVOID ServerPortCookie, 
    PVOID Context, 
    ULONG ContextSize, 
    PVOID ConnectionCookie
) {
    ClientPortGlobal = ClientPort;
}

VOID FsAntiScamDisconnect(
    PVOID ConnectionCookie
) {
    FltCloseClientPort(
        FilterHandle,
        &ClientPortGlobal);
}

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

// Entry point for the minifilter.
NTSTATUS DriverEntry(
    PDRIVER_OBJECT DriverObject, 
    PUNICODE_STRING RegistryPath
) {
    KdPrint(("Driver is loading!"));
    NTSTATUS status;
    // MF Communication stuff
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    UNICODE_STRING PortNameString = RTL_CONSTANT_STRING(PortName);

    UNREFERENCED_PARAMETER(RegistryPath);

    // Register our minifilter with the OS
    status = FltRegisterFilter(
        DriverObject, 
        &FilterRegistration, 
        &FilterHandle
    );

    // Usermode <-> Kernel communication
    status = FltBuildDefaultSecurityDescriptor(
        &SecurityDescriptor, 
        FLT_PORT_ALL_ACCESS
    );
   
    if (NT_SUCCESS(status)) {
        InitializeObjectAttributes(
            &ObjectAttributes, 
            &PortNameString, 
            OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 
            NULL, 
            SecurityDescriptor
        );
        status = FltCreateCommunicationPort(
            FilterHandle, 
            &Port, 
            &ObjectAttributes, 
            NULL, 
            FsAntiScamConnect, 
            FsAntiScamDisconnect, 
            FsAntiScamMessageReceived, 
            1
        );
        KdPrint(("Port Opened! \r\n"));
        FltFreeSecurityDescriptor(SecurityDescriptor);
        
        if (NT_SUCCESS(status)) {
            KdPrint(("Filtering now!\r\n"));
            status = FltStartFiltering(FilterHandle);

            if (!NT_SUCCESS(status)) {
                KdPrint(("Error in DriverEntry. Killing program! \r\n"));
                FltUnregisterFilter(FilterHandle);
                FltCloseCommunicationPort(Port);
            }
        }
    }
    return status;
}