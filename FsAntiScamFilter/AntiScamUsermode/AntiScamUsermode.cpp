#include <iostream>
#include <windows.h>
#include <fltUser.h>
#include "..\FsAntiScamFilter\FilterCommunication.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "fltlib.lib")

HANDLE Port = NULL;

int main()
{
    std::cout << "AntiScam usermode client\n";
    std::cout << "Attempting to establish connection with filter port "<< PortName << "\n";

    DWORD BytesReceived = 0;
    PCHAR MessageBuffer = "Message";
    char ReceiveBuffer[500] = { 0 };

    if (Port == NULL) {
        HRESULT result = FilterConnectCommunicationPort(PortName, 0, NULL, 0, NULL, &Port);
        std::cout << "Result code: " << result << "PortName " << PortName << "Port: " << Port;
        if (FAILED(result)) {
            std::cout << "Couldn't connect to filter!";
            return 0;
         }
    }
    if (FilterSendMessage(Port, MessageBuffer, strlen(MessageBuffer), ReceiveBuffer, 500, &BytesReceived)) {
        std::cout << ReceiveBuffer << "\n";
    }

    return 0;
}