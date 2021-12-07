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
#include "../AntiScamUsermode/CustomIOCTL.h"

PFLT_FILTER FilterHandle = NULL;
PDRIVER_OBJECT DriverObjGlobal = NULL;

const PUNICODE_STRING DEVICE_NAME = L"\\Device\\FsAntiScam";
const PUNICODE_STRING DEVICE_SYMBOLIC_LINK = L"\\??\\FsAntiScamLink";

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
    FltUnregisterFilter(FilterHandle);
    if (DriverObjGlobal != NULL) {
        IoDeleteDevice(DriverObjGlobal->DeviceObject);
        IoDeleteSymbolicLink(&DEVICE_SYMBOLIC_LINK);
    }

    return STATUS_SUCCESS;
}

NTSTATUS MajorFunctionHandler(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION stackLocation = NULL;
    stackLocation = IoGetCurrentIrpStackLocation(Irp);

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS CustomIOCTLHandler(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    PIO_STACK_LOCATION stackLocation = NULL;
    stackLocation = IoGetCurrentIrpStackLocation(Irp);

    if (stackLocation->Parameters.DeviceIoControl.IoControlCode == FsAntiScam_IOCTL_HELLO) {
        CHAR* message = "Hello! :)";
        Irp->IoStatus.Information = strlen(message);
        Irp->IoStatus.Status = STATUS_SUCCESS;

        RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, message, strlen(Irp->AssociatedIrp.SystemBuffer));

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_SUCCESS;
    }

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_BAD_DATA; // Use this to indicate we don't know what that IOCTL call is

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

// Entry point for the minifilter.
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    NTSTATUS status;

    UNREFERENCED_PARAMETER(RegistryPath);

    DriverObjGlobal = DriverObject;

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = CustomIOCTLHandler;

    DriverObject->MajorFunction[IRP_MJ_CREATE] = MajorFunctionHandler;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = MajorFunctionHandler;

    status = IoCreateDevice(DriverObject, 0, &DEVICE_NAME, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DriverObject->DeviceObject);

    if (!NT_SUCCESS(status)) {
        KdPrint(("[FsAntiScamFilter] Could not create device! \r\n"));
    }

    status = IoCreateSymbolicLink(&DEVICE_SYMBOLIC_LINK, &DEVICE_NAME);
    if (!NT_SUCCESS(status)) {
        KdPrint(("[FsAntiScamFilter] Could not create link! \r\n"));
    }
    status = FltRegisterFilter(DriverObject, &FilterRegistration, &FilterHandle);
   
    if (NT_SUCCESS(status)) {
        status = FltStartFiltering(FilterHandle);

        if (!NT_SUCCESS(status)) {
            FltUnregisterFilter(FilterHandle);
        }
    }

    return status;
}