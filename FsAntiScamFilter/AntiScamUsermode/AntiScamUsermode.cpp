#include <iostream>
#include <string>
#include <windows.h>
#include <fltUser.h>
#include <WinUser.h>
#include <chrono>
#include <thread>
#include "..\FsAntiScamFilter\FilterCommunication.h"

void CheckForNewKernelMessages();
void RunCommandFromKernel(std::string KernelMessage);

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "fltlib.lib")

HANDLE Port = INVALID_HANDLE_VALUE;

int main()
{
    HRESULT result = S_OK;

    std::cout << "AntiScam usermode client\n";
    std::cout << "Attempting to establish connection with filter port...\n";

    DWORD BytesReceived = 0;
    PCHAR MessageBuffer = "Message";
    char ReceiveBuffer[500] = { 0 };

    result = FilterConnectCommunicationPort(PortName, 0, NULL, 0, NULL, &Port);
    if (IS_ERROR(result)) {
        printf("Could not connect to filter! :( - 0x%08x\n", result);
        return 0;
    }
    printf("Connected to filter!\n");
    printf("--------------------\n");

    bool isRunning = true;
    while (isRunning) {
        CheckForNewKernelMessages();
        std::this_thread::sleep_for(std::chrono::seconds(10)); // We don't need to spam the kernel.
    }
    return 0;
}
    
void CheckForNewKernelMessages() {
    PCHAR Command = "FILTER_GET_NEW_MESSAGES";
    DWORD BytesReceivers = 0;
    char ReceiverBuffer[500] = { 0 };
    HRESULT message = FilterSendMessage(Port, Command, strlen(Command), ReceiverBuffer, 500, &BytesReceivers);
    if (IS_ERROR(message)) {
        printf("Error sending command to filter! Error: 0x%08x\n", message);
    }
    std::string kernelMessage(ReceiverBuffer);
    printf("Kernel said: %s\n", kernelMessage.c_str());
    RunCommandFromKernel(kernelMessage);
}

// Sidenote: This code is quite bad. If there's a better way, pray tell.
void RunCommandFromKernel(std::string KernelMessage) {
    // Filter wanted a messagebox created
    if (KernelMessage.find("FILTER_CREATE_MESSAGE_BOX") != std::string::npos) {
        std::string message = KernelMessage.substr(25);
        std::wstring messageWstr = std::wstring(message.begin(), message.end());
        LPCWSTR mBoxMessage = messageWstr.c_str(); // Get string *after* the message
        MessageBox(NULL,
            mBoxMessage,
            L"Notice from AntiScam filter!",
            MB_OK);
    }
    if (KernelMessage.find("FILTER_STANDARD_LOG") != std::string::npos) {
        std::string message = KernelMessage.substr(19);
        printf("[NORMAL] %s", message.c_str());
    }
    if (KernelMessage.find("FILTER_WARNING_LOG") != std::string::npos) {
        std::string message = KernelMessage.substr(18);
        printf("[WARNING] %s", message.c_str());
    }
    // error log
    // settings
}

void SendCommandToKernel(std::string message) {

}