/*++

Module Name:

    FsAntiScamFilter.c

Abstract:

    This is the main module of the FsAntiScamFilter miniFilter driver.

Environment:

    Kernel mode

--*/

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include "FilterCommunication.h"

// Communication
PFLT_FILTER FilterHandle = NULL;
PFLT_PORT Port = NULL;
PFLT_PORT ClientPortGlobal = NULL;
// Generic functions
NTSTATUS FsFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);
// IRP_MJ_CREATE
FLT_PREOP_CALLBACK_STATUS FsFilterPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext);
FLT_POSTOP_CALLBACK_STATUS FsFilterPostCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags);
// IRP_MJ_READ
FLT_PREOP_CALLBACK_STATUS FsFilterPreRead(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext);
FLT_POSTOP_CALLBACK_STATUS FsFilterPostRead(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags);
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

FLT_PREOP_CALLBACK_STATUS FsFilterPreRead(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {

    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(FltObjects);
    
    PFLT_FILE_NAME_INFORMATION FileNameInfo;
    NTSTATUS status;
    WCHAR FileName[400] = { 0 };

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
    ULONG CallingId = FltGetRequestorProcessId(Data);

    if (NT_SUCCESS(status)) {
        status = FltParseFileNameInformation(FileNameInfo);
        if (NT_SUCCESS(status)) {
            if (FileNameInfo->Name.MaximumLength < 400) {
                RtlCopyMemory(FileName, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
                _wcsupr(FileName);
                // KdPrint(("File PreRead: %ws \r\n", FileName)); // Spammy af line
                if (wcsstr(FileName, L"CANTREADME.TXT") != NULL) {
                    KdPrint(("Can't read that! (denied) \r\n"));
                    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                    Data->IoStatus.Information = 0;
                    FltReleaseFileNameInformation(FileNameInfo);
                    return FLT_PREOP_COMPLETE;
                }
            }
        }
    }

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS FsFilterPostRead(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags) {
   
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Data);

    return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_POSTOP_CALLBACK_STATUS FsFilterPostCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags) {
    
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Data);
    
    return FLT_POSTOP_FINISHED_PROCESSING;
}


// PreCreate routine handler
FLT_PREOP_CALLBACK_STATUS FsFilterPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {

    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(FltObjects);

    PFLT_FILE_NAME_INFORMATION FileNameInfo;
    NTSTATUS status;
    WCHAR FileName[400] = { 0 };

    PUNICODE_STRING CallingProcessName[400] = { 0 };
    
    PEPROCESS CallingProcess = FltGetRequestorProcess(Data);
    status = SeLocateProcessImageName(CallingProcess, &CallingProcessName);

    if (!NT_SUCCESS(status)) {
        KdPrint(("crap"));
    }

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);


    if (NT_SUCCESS(status)) {
        status = FltParseFileNameInformation(FileNameInfo);

        if (NT_SUCCESS(status)) {
            if (FileNameInfo->Name.MaximumLength < 400) {
                RtlCopyMemory(FileName, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
                //KdPrint(("File PreCreate: %ws \r\n", FileName));
                _wcsupr(FileName);
                if (wcsstr(FileName, L"CANTWRITETOME.TXT") != NULL) {
                    KdPrint(("Can't write that! (denied) \r\n"));
                    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                    Data->IoStatus.Information = 0;
                    FltReleaseFileNameInformation(FileNameInfo);
                    return FLT_PREOP_COMPLETE;
                }
            }
        }
        FltReleaseFileNameInformation(FileNameInfo);
    }

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

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

VOID FsAntiScamDisconnect(PVOID ConnectionCookie) {
    FltCloseClientPort(FilterHandle, &ClientPortGlobal);
}

NTSTATUS FsAntiScamMessageReceived(
    PVOID PortCookie, 
    PVOID InputBuffer, 
    ULONG InputBufferLength, 
    PVOID OutputBuffer, 
    ULONG OutputBufferLength, 
    PULONG ReturnBufferLength
) {
    PCHAR message = "Hello! :)";

    PCHAR ApplicationMessage = (PCHAR)InputBuffer;

    KdPrint(("Application message is: %s \r\n", ApplicationMessage));

    strcpy((PCHAR)OutputBuffer, message);
    return STATUS_SUCCESS;
}

// Entry point for the minifilter.
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    NTSTATUS status;
    // MF Communication stuff
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    UNICODE_STRING PortNameString = RTL_CONSTANT_STRING(L"\\FsAntiScamPort");

    UNREFERENCED_PARAMETER(RegistryPath);

    // Register our minifilter with the OS
    status = FltRegisterFilter(DriverObject, &FilterRegistration, &FilterHandle);

    // Usermode<->Kernel communication
    status = FltBuildDefaultSecurityDescriptor(&SecurityDescriptor, FLT_PORT_ALL_ACCESS);
   
    if (NT_SUCCESS(status)) {

        InitializeObjectAttributes(&ObjectAttributes, &PortNameString, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, SecurityDescriptor);
        status = FltCreateCommunicationPort(FilterHandle, &Port, &ObjectAttributes, NULL, FsAntiScamConnect, FsAntiScamDisconnect, FsAntiScamMessageReceived, 1);
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