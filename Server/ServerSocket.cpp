#include "ServerSocket.hpp"

ServerSocket::ServerSocket() {
    WSADATA wsaSATA;
    if (WSAStartup(MAKEWORD(2, 2), &wsaSATA) != 0) {
        std::cerr << "WSAStartup failed.\n";
        WSACleanup();
        exit(1);
    }
    
    // Creating server socket
    serverSocket = static_cast<int>(socket(AF_INET, SOCK_STREAM, 0));
    if (serverSocket == -1) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        exit(1);
    }

    serverAddress.sin_family = AF_INET;
    InetPton(AF_INET, _T(SERVER_IP), &serverAddress.sin_addr.s_addr);
    serverAddress.sin_port = htons(PORT);

    // Bind
    if (bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << '\n';
        WSACleanup();
        exit(1);
    }

    // Listen to connection from client
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << '\n';
        WSACleanup();
        exit(1);
    }

    std::cout << "Server is listening on port " << PORT << "...\n";

    WinAPI::initializeGDIPlus();
    initializeHandlers();
}

MessageType ServerSocket::hashMessage(const std::string& message) {

    static const std::unordered_map<std::string, MessageType> 
        messageMap = {

            {"shutdown", SHUTDOWN},
            {"restart", RESTART},
            {"get IP", GET_IP},
            {"Hello server", HELLO_SERVER},
            {"STOP", STOP},
            {"capture screen", CAPTURE_SCREEN},
            {"copy file", COPY_FILE},
            {"delete file", DELETE_FILE},
            {"create folder", CREATE_FOLDER},
            {"copy folder", COPY_FOLDER}

        };

    auto it = messageMap.find(message);

    return (it != messageMap.end() ? it->second : INVALID);
}

void ServerSocket::initializeHandlers()
{
    handlers[SHUTDOWN] = [](SOCKET&) { 
        WinAPI::systemShutdown(); 
    };

    handlers[RESTART] = [](SOCKET&) { 
        LPWSTR restartMessage = L"RESTART"; 
        WinAPI::systemRestart(restartMessage); 
    };

    handlers[GET_IP] = [this](SOCKET& clientSocket) { 
        this->sendIPAddress(clientSocket); 
    };

    handlers[HELLO_SERVER] = [this](SOCKET& clientSocket) {
        std::string response = "Hello from server!";
        this->sendMessage(clientSocket, response.c_str());
    };

    handlers[STOP] = [this](SOCKET& clientSocket) {
        this->~ServerSocket();
        closesocket(clientSocket);
        exit(0);
    };

    handlers[CAPTURE_SCREEN] = [this](SOCKET& clientSocket) {
        std::string result = WinAPI::saveScreenshot();
        std::string response = (result.substr(0, 6) != "Failed") ? "Capture screen successful, new file: " + result : "Error: " + result;
        this->sendMessage(clientSocket, response.c_str());
    };

    handlers[COPY_FILE] = [this](SOCKET& clientSocket) {
        const wchar_t* source = L"source.txt";
        const wchar_t* destination = L"destination.txt";
        std::string result = WinAPI::copyFile(source, destination);
        std::string response = (result.substr(0, 6) != "Failed") ? result : "Error: " + result;
        this->sendMessage(clientSocket, response.c_str());
    };

    handlers[DELETE_FILE] = [](SOCKET&) { 
        const wchar_t* fileToDelete = L"file.txt"; 
        WinAPI::deleteFile(fileToDelete); 
    };

    handlers[CREATE_FOLDER] = [](SOCKET&) { 
        const wchar_t* folderPath = L"new_folder"; 
        WinAPI::createFolder(folderPath); 
    };

    handlers[COPY_FOLDER] = [](SOCKET&) { 
        const wchar_t* sourceFolder = L"source_folder"; 
        const wchar_t* destinationFolder = L"destination_folder"; 
        WinAPI::copyFolder(sourceFolder, destinationFolder); 
    };
}

void ServerSocket::handleEvent(SOCKET &clientSocket, const std::string& message) {
    MessageType messageType = hashMessage(message);
    auto it = handlers.find(messageType);
    if (it != handlers.end()) {
        it->second(clientSocket);
    } else {
        std::string invalidMessage = "INVALID MESSAGE";
        send(clientSocket, invalidMessage.c_str(), static_cast<int>(invalidMessage.length()), 0);
    }
}
