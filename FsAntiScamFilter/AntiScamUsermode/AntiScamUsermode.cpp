#include <iostream>
#include <string>
#include <windows.h>
#include <fltUser.h>
#include <WinUser.h>
#include <chrono>
#include <thread>
#include "..\FsAntiScamFilter\FilterCommunication.h"
#include <wintoast/wintoastlib.h>
#include "ProgramUtils.h"

using namespace WinToastLib;
// Define toast notification handler
class CustomHandler : public IWinToastHandler {
public:
    void toastActivated() const {
        std::wcout << L"The user clicked in this toast" << std::endl;
        exit(0);
    }

    void toastActivated(int actionIndex) const {
        std::wcout << L"The user clicked on action #" << actionIndex << std::endl;
        exit(16 + actionIndex);
    }

    void toastDismissed(WinToastDismissalReason state) const {
        switch (state) {
        case UserCanceled:
            std::wcout << L"The user dismissed this toast" << std::endl;
            exit(1);
            break;
        case TimedOut:
            std::wcout << L"The toast has timed out" << std::endl;
            exit(2);
            break;
        case ApplicationHidden:
            std::wcout << L"The application hid the toast using ToastNotifier.hide()" << std::endl;
            exit(3);
            break;
        default:
            std::wcout << L"Toast not activated" << std::endl;
            exit(4);
            break;
        }
    }

    void toastFailed() const {
        std::wcout << L"Error showing current toast" << std::endl;
        exit(5);
    }
};

void CheckForNewKernelMessages();
void RunCommandFromKernel(std::string KernelMessage);

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "fltlib.lib")

HANDLE Port = INVALID_HANDLE_VALUE;

int main()
{
    // Program needs to be elevated in order to connect to filter port
    if (!IsElevated()) {
        printf("Program is not running as administrator! You must run it as administrator or it cannot connect to the filter.\n");
        exit(1);
    }
    HRESULT result = S_OK;

    SetConsoleTitle(L"AntiScam Usermode Client");
    std::cout << "AntiScam usermode client\n";
    std::cout << "Attempting to establish connection with filter port...\n";

    if (!WinToast::isCompatible()) {
        printf("Your computer is not compatible with WinToast! You will not get desktop notifications.\n");
    }

    WinToast::instance()->setAppName(L"AntiScamClient");
    const auto aumi = WinToast::configureAUMI(L"Whanos", L"AntiScamClient", L"AntiScamClient", L"2021112");
    WinToast::instance()->setAppUserModelId(aumi);

    if (!WinToast::instance()->initialize()) {
        printf("Error: Could not initialise WinToast lib!\n");
    }

    CustomHandler* handler = new CustomHandler;
    WinToastTemplate templ = WinToastTemplate(WinToastTemplate::Text01);
    templ.setTextField(L"Discord AntiScam is running!", WinToastTemplate::FirstLine);

    if (!WinToast::instance()->showToast(templ, handler)) {
        printf("It borked\n");
    }
    
    // Start our buffers
    DWORD BytesReceived = 0;
    char ReceiveBuffer[500] = { 0 };

    result = FilterConnectCommunicationPort(PortName, 0, NULL, 0, NULL, &Port);
    if (IS_ERROR(result)) {
        printf("Could not connect to filter! :( - 0x%08x\n", result);
        return 0;
    }

    system("cls"); // Clear text
    printf("Connected to filter!\n");
    printf("--------------------\n");

    bool isRunning = true;
    while (isRunning) {
        CheckForNewKernelMessages();
        // std::this_thread::sleep_for(std::chrono::seconds(10)); // We don't need to spam the kernel.
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
        std::string message = KernelMessage.substr(26);
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