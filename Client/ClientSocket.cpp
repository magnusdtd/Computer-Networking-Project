#include "ClientSocket.hpp"

ClientSocket::ClientSocket() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        std::cerr << "The status: " << wsaData.szSystemStatus << "\n";
        WSACleanup();
        exit(1);
    }
    
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        exit(1);
    }

    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(PORT);
    InetPton(AF_INET, _T(SERVER_IP), &clientAddress.sin_addr.s_addr);

    if (connect(clientSocket, (SOCKADDR*)&clientAddress, sizeof(clientAddress)) == SOCKET_ERROR) {
        std::cerr << "Connection to server failed.\n";
        closesocket(clientSocket);
        WSACleanup();
        exit(1);
    }
}

void ClientSocket::receiveFile(const std::string &filePath)
{
    // Ensure the directory exists
    std::filesystem::path dir = std::filesystem::path(filePath).parent_path();
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }

    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " + filePath + " for writing.\n";
        return;
    }

    // Receive file size
    int fileSizeInt;
    recv(clientSocket, reinterpret_cast<char*>(&fileSizeInt), sizeof(fileSizeInt), 0);
    std::streamsize fileSize = static_cast<std::streamsize>(fileSizeInt);

    // Receive file content
    char buffer[1024];
    std::streamsize totalBytesReceived = 0;
    while (totalBytesReceived < fileSize) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::cerr << "Error: Failed to receive file content.\n";
            break;
        }
        outFile.write(buffer, bytesReceived);
        totalBytesReceived += bytesReceived;
    }

    outFile.close();
    std::cout << "File received successfully.\n"; 
}
