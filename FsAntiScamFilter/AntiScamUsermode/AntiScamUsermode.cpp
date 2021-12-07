#include <iostream>
#include <windows.h>
#include <fltUser.h>
#include "..\FsAntiScamFilter\FilterCommunication.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "fltlib.lib")

int main()
{
    HANDLE Port = INVALID_HANDLE_VALUE;
    HRESULT result = S_OK;

    std::cout << "AntiScam usermode client\n";
    std::cout << "Attempting to establish connection with filter port...\n";

    DWORD BytesReceived = 0;
    PCHAR MessageBuffer = "Message";
    char ReceiveBuffer[500] = { 0 };

    result = FilterConnectCommunicationPort(L"\\FsAntiScamPort", 0, NULL, 0, NULL, &Port);
    if (IS_ERROR(result)) {
        printf("Could not connect to filter! :( - 0x%08x\n", result);
        return 0;
    }
    printf("Connected to filter!\n");
    HRESULT message = FilterSendMessage(Port, MessageBuffer, strlen(MessageBuffer), ReceiveBuffer, 500, &BytesReceived);
    if (SUCCEEDED(message)) {
        printf("%s", ReceiveBuffer);
    }
    return 0;
}