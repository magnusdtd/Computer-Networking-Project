#ifndef CLIENT_SOCKET_HPP
#define CLIENT_SOCKET_HPP

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <tchar.h>

#include <fstream>
#include <filesystem>
#include <unordered_map>

#define PORT 8080
#define SERVER_IP "127.0.0.1"

extern std::unordered_map<std::string, std::string> messageMap;

class ClientSocket {
private:
    SOCKET clientSocket;
    sockaddr_in clientAddress;
    char buffer[1024];
    int bytesReceived;
    bool stopClient;
public:
    ClientSocket();

    ~ClientSocket() {
        closesocket(clientSocket);
        WSACleanup();
    }

    SOCKET getSocket() {
        return clientSocket;
    }

    bool getStopClient() { return stopClient; }

    void receiveFile(const std::string& filePath);

    bool executeCommand(const std::string &command, std::string &response, std::string& filePath);
    
};

#endif