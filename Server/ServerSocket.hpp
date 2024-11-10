#pragma once

#include <iostream>
#include <unordered_map>
#include <functional>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#pragma comment(lib, "ws2_32.lib")

#include "./../WindowAPI/winAPI.hpp"

#define PORT 8080
#define SERVER_IP "127.0.0.1"

enum MessageType {
    SHUTDOWN,
    RESTART,
    GET_IP,
    HELLO_SERVER,
    STOP,
    INVALID,
    CAPTURE_SCREEN,
    COPY_FILE,
    DELETE_FILE,
    CREATE_FOLDER,
    COPY_FOLDER,
    LIST_COMMANDS,
    LIST_PROCESS,
    LIST_SERVICES,
    START_APP,
    TERMINATE_PROCESS,
    LIST_APP,

};

class ServerSocket {
private:
    SOCKET serverSocket;
    sockaddr_in serverAddress;

    static std::unordered_map<std::string, MessageType> messageMap;

    std::unordered_map<MessageType, std::function<void(SOCKET&, const std::string& command)>> handlers;

    MessageType hashMessage(const std::string message);

    void initializeHandlers();

    WinAPI winAPI;

    std::string response;

public:
    ServerSocket();

    ~ServerSocket() {
        closesocket(serverSocket);
        WSACleanup();
    }

    void sendIPAddress(SOCKET &clientSocket) {
        const char* ipAddress = SERVER_IP;
        send(clientSocket, ipAddress, static_cast<int>(strlen(ipAddress)), 0);
        std::cout << "Sending IP address: " << ipAddress << '\n';
    }

    void sendMessage(SOCKET &clientSocket, const char* message) {
        send(clientSocket, message, static_cast<int>(strlen(message)), 0);
    }

    SOCKET getSocket() {
        return serverSocket;
    }

    void handleEvent(SOCKET &clientSocket, const std::string& message);

    
};