#include "ClientSocket.hpp"


std::unordered_map<std::string, std::string> messageMap = {
    {"shutdown", "SHUTDOWN"},
    {"restart", "RESTART"},
    {"getIP", "GET_IP"},
    {"HelloServer", "HELLO_SERVER"},
    {"STOP", "STOP"},
    {"captureScreen", "CAPTURE_SCREEN"},
    {"copy", "COPY_FILE"},
    {"delete", "DELETE_FILE"},
    {"createFolder", "CREATE_FOLDER"},
    {"copyFolder", "COPY_FOLDER"},
    {"ls", "LIST_COMMANDS"},
    {"listProcess", "LIST_PROCESS"},
    {"listService", "LIST_SERVICES"},
    {"startApp", "START_APP"},
    {"terminateProcess", "TERMINATE_PROCESS"},
    {"listRunningApp", "LIST_RUNNING_APP"},
    {"listInstalledApp", "LIST_INSTALLED_APP"},
    {"listFiles", "LIST_FILES"},
    {"disableKeyboard", "DISABLE_KEYBOARD"},
    {"enableKeyboard", "ENABLE_KEYBOARD"},
    {"enableKeylogger", "ENABLE_KEYLOGGER"},
    {"disableKeylogger", "DISABLE_KEYLOGGER"},
    {"screenRecording", "SCREEN_RECORDING"}
};

ClientSocket::ClientSocket(): bytesReceived(0), stopClient(false) {
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

bool ClientSocket::executeCommand(const std::string &command, std::string &response, std::string& filePath)
{
    send(clientSocket, command.c_str(), static_cast<int>(command.length()), 0);

    if (command == "STOP") {
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        stopClient = true;
        return true;
    }
    else if (command == "listProcess" ||
        command == "listService" ||
        command == "listRunningApp" ||
        command == "listInstalledApp" ||
        command == "listFiles" ||
        command == "captureScreen" ||
        command == "disableKeylogger" ) {
        // Receive the file name from the server
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "\t-> [Response from server] " << buffer << '\n';

            // Extract the file path from the server's response command
            std::string temp(buffer); response = temp;
            std::string filePath;
            std::string keyword = " at ";
            size_t pos = temp.find(keyword);
            filePath = temp.substr(pos + keyword.length());

            // Extract the file name from the file path
            std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
            std::string outputFilePath = "./output-client/" + fileName;
            receiveFile(outputFilePath);

            return true;
        } else {
            std::cerr << "\t-> [Client] Failed to receive file name from the server.\n";
            return false;
        }
    } else {
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "\t-> [Response from server] " << buffer << '\n';

            std::string temp(buffer); response = temp;

            return true;
        } else {
            std::cerr << "\t-> [Client] Failed to receive file name from the server.\n";
            return false;
        }
    }
}
