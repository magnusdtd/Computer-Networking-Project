#pragma once

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <tchar.h>

#include <fstream>
#include <filesystem>

#define PORT 8080
#define SERVER_IP "127.0.0.1"

class ClientSocket {
private:
    SOCKET clientSocket;
    sockaddr_in clientAddress;
public:
    ClientSocket();

    ~ClientSocket() {
        closesocket(clientSocket);
        WSACleanup();
    }

    SOCKET getSocket() {
        return clientSocket;
    }

    void receiveFile(const std::string& filePath);
};