#include "DriverHeaders.h"

// Undocumented windows bs
extern UCHAR* PsGetProcessImageFileName(IN PEPROCESS Process);

extern NTSTATUS PsLookupProcessByProcessId(
    HANDLE ProcessId,
    PEPROCESS* Process
);
typedef PCHAR(*GET_PROCESS_IMAGE_NAME) (PEPROCESS Process);
GET_PROCESS_IMAGE_NAME gGetProcessImageFileName;

char* GetProcessNameFromPid(HANDLE pid)
{
    PEPROCESS Process;
    if (PsLookupProcessByProcessId(pid, &Process) == STATUS_INVALID_PARAMETER)
    {
        return "pid???";
    }
    return (CHAR*)PsGetProcessImageFileName(Process);
}

FLT_PREOP_CALLBACK_STATUS FsFilterPreRead(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext
) {
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(FltObjects);

    PFLT_FILE_NAME_INFORMATION FileNameInfo;
    NTSTATUS status;
    WCHAR FileName[400] = { 0 };
    PUNICODE_STRING CallingProcessPath = NULL; // Returns as volume
    PEPROCESS CallingProcess = FltGetRequestorProcess(Data);

    // 
    status = SeLocateProcessImageName(CallingProcess, &CallingProcessPath);
    int processId = PsGetProcessId(CallingProcess);
    char* Path = GetProcessNameFromPid(processId);

    if (!NT_SUCCESS(status)) {
        KdPrint(("SeLocateProcessImageName failed! (line 185) %lx\n", status));
    }

    status = FltGetFileNameInformation(
        Data,                                                   // PFLT_CALLBACK_DATA
        FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, // FLT_FILE_NAME_OPTIONS
        &FileNameInfo                                           // PFLT_FILE_NAME_INFORMATION
    );

    if (NT_SUCCESS(status)) {
        status = FltParseFileNameInformation(FileNameInfo);
        if (NT_SUCCESS(status)) {
            if (FileNameInfo->Name.MaximumLength < 400) {
                RtlCopyMemory(FileName, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
                _wcsupr(FileName);
                // KdPrint(("File PreRead: %ws \r\n", FileName)); // Spammy af line
                if (wcsstr(FileName, L"DISCORD\\LOCAL STORAGE\\LEVELDB") != NULL) {
                    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                    Data->IoStatus.Information = 0;
                    KdPrint(("%wZ tried to read %ws \r\n", CallingProcessPath, FileName));
                    FltReleaseFileNameInformation(FileNameInfo);
                    return FLT_PREOP_COMPLETE;
                }
            }
        }
    }

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS FsFilterPostRead(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext,
    FLT_POST_OPERATION_FLAGS Flags
) {

    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Data);

    return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_POSTOP_CALLBACK_STATUS FsFilterPostCreate(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext,
    FLT_POST_OPERATION_FLAGS Flags
) {

    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Data);

    return FLT_POSTOP_FINISHED_PROCESSING;
}


// PreCreate routine handler
FLT_PREOP_CALLBACK_STATUS FsFilterPreCreate(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext
) {
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(FltObjects);

    PFLT_FILE_NAME_INFORMATION FileNameInfo;
    NTSTATUS status;
    WCHAR FileName[512] = { 0 };
    PUNICODE_STRING CallingProcessPath = NULL; // Returns as volume
    PEPROCESS CallingProcess = FltGetRequestorProcess(Data);

    // 
    status = SeLocateProcessImageName(CallingProcess, &CallingProcessPath);
    int processId = PsGetProcessId(CallingProcess);
    char* Path = GetProcessNameFromPid(processId);

    if (!NT_SUCCESS(status)) {
        KdPrint(("SeLocateProcessImageName failed! (line 185) %lx\n", status));
    }

    status = FltGetFileNameInformation(
        Data,                                                   // PFLT_CALLBACK_DATA
        FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, // FLT_FILE_NAME_OPTIONS
        &FileNameInfo                                           // PFLT_FILE_NAME_INFORMATION
    );

    if (NT_SUCCESS(status)) {
        status = FltParseFileNameInformation(FileNameInfo);

        if (NT_SUCCESS(status)) {
            if (FileNameInfo->Name.MaximumLength < 400) {
                RtlCopyMemory(FileName, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
                // KdPrint(("%wZ Caused File PreCreate: %ws \r\n", CallingProcessName, FileName)); // Spammy af line
                _wcsupr(FileName);
                // Check for processes trying to read LEVELDB files
                if (wcsstr(FileName, L"DISCORD\\LOCAL STORAGE\\LEVELDB") != NULL) {
                    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                    Data->IoStatus.Information = 0;
                    KdPrint(("%wZ tried to read %ws \r\n", CallingProcessPath, FileName));
                    FltReleaseFileNameInformation(FileNameInfo);
                    return FLT_PREOP_COMPLETE;
                }
                // Check for processes trying to write to 
                if (wcsstr(FileName, L"DISCORD_DESKTOP_CORE") != NULL) {
                    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                    Data->IoStatus.Information = 0;
                    KdPrint(("%wZ tried to read %ws \r\n", CallingProcessPath, FileName));
                    FltReleaseFileNameInformation(FileNameInfo);
                    return FLT_PREOP_COMPLETE;
                }
            }
        }
        FltReleaseFileNameInformation(FileNameInfo);
    }

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}