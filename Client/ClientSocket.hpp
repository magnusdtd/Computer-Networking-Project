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
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "./../GmailAPI/GmailAPI.hpp"

#define PORT 8080
#define SERVER_IP "127.0.0.1"

extern std::unordered_map<std::string, std::string> messageMap;

class ClientSocket : public GmailAPI {
private:
    SOCKET clientSocket;
    sockaddr_in clientAddress;
    char buffer[1024];
    int bytesReceived;
    bool stopClient;

    std::queue<User*> userQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondVar;
    std::thread messageQueueThread;
    bool isStopMQThread;

public:
    ClientSocket(const std::string& oauthFilePath, const std::string& tokenFilePath, const std::string& scriptFilePath, const std::string& messageListFilePath);

    ~ClientSocket();

    SOCKET getSocket() {
        return clientSocket;
    }

    bool getStopClient() { return stopClient; }

    void receiveFile(const std::string& filePath);

    bool executeCommand(const std::string &command, std::string &response, std::string& filePath);

    void processQueue();

    void fetchMessageDetails(CURL *curl, const std::string &messageUrl, std::string &readBuffer, std::ofstream &file) override;
};

#endif