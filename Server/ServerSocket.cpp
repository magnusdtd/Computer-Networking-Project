#include "ServerSocket.hpp"

std::unordered_map<std::string, MessageType> ServerSocket::messageMap;

ServerSocket::ServerSocket() : winAPI() {
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

    initializeHandlers();

    messageMap = {
        {"shutdown", SHUTDOWN},
        {"restart", RESTART},
        {"getIP", GET_IP},
        {"HelloServer", HELLO_SERVER},
        {"STOP", STOP},
        {"captureScreen", CAPTURE_SCREEN},
        {"copy", COPY_FILE},
        {"delete", DELETE_FILE},
        {"createFolder", CREATE_FOLDER},
        {"copyFolder", COPY_FOLDER},
        {"ls", LIST_COMMANDS},
        {"listProcess", LIST_PROCESS},
        {"listServices", LIST_SERVICES}
    };
}

MessageType ServerSocket::hashMessage(const std::string message) {
    std::istringstream iss(message);
    std::string command;
    iss >> command;

    auto it = messageMap.find(command);
    return (it != messageMap.end() ? it->second : INVALID);
}

void ServerSocket::initializeHandlers()
{
    handlers[SHUTDOWN] = [this](SOCKET&, const std::string& command) { 
        winAPI.systemShutdown(); 
    };

    handlers[RESTART] = [this](SOCKET&, const std::string& command) { 
        LPWSTR restartMessage = L"RESTART"; 
        winAPI.systemRestart(restartMessage); 
    };

    handlers[GET_IP] = [this](SOCKET& clientSocket, const std::string& command) { 
        this->sendIPAddress(clientSocket); 
    };

    handlers[HELLO_SERVER] = [this](SOCKET& clientSocket, const std::string& command) {
        std::string response = "Hello from server!";
        this->sendMessage(clientSocket, response.c_str());
    };

    handlers[STOP] = [this](SOCKET& clientSocket, const std::string& command) {
        this->~ServerSocket();
        closesocket(clientSocket);
        exit(0);
    };

    handlers[CAPTURE_SCREEN] = [this](SOCKET& clientSocket, const std::string& command) {
        std::string result = winAPI.saveScreenshot();
        std::string response = (result.substr(0, 6) != "Failed") ? "Capture screen successful, new file: " + result : "Error: " + result;
        this->sendMessage(clientSocket, response.c_str());
    };

    handlers[COPY_FILE] = [this](SOCKET& clientSocket, const std::string& command) {
        std::istringstream iss(command);
        std::vector<std::string> tokens{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};

        if (tokens.size() != 3) {
            std::string guide = "Usage: copy <source_path> <destination_path>";
            this->sendMessage(clientSocket, guide.c_str());
            return;
        }

        const std::wstring source = std::wstring(tokens[1].begin(), tokens[1].end());
        const std::wstring destination = std::wstring(tokens[2].begin(), tokens[2].end());

        std::string result = winAPI.copyFile(source.c_str(), destination.c_str());
        std::string response = (result.substr(0, 6) != "Failed") ? result : "Error: " + result;
        this->sendMessage(clientSocket, response.c_str());
    };

    handlers[DELETE_FILE] = [this](SOCKET &clientSocket, const std::string& command) { 
        std::istringstream iss(command);
        std::vector<std::string> tokens{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};

        if (tokens.size() != 2) {
            std::string guide = "Usage: delete <source_path>";
            this->sendMessage(clientSocket, guide.c_str());
            return;
        }

        const std::wstring source = std::wstring(tokens[1].begin(), tokens[1].end());

        std::string result = winAPI.deleteFile(source.c_str());
        std::string response = (result.substr(0, 6) != "Failed") ? result : "Error: " + result;
        this->sendMessage(clientSocket, response.c_str());   
    };

    handlers[CREATE_FOLDER] = [this](SOCKET& clientSocket, const std::string& command) {
        std::istringstream iss(command);
        std::vector<std::string> tokens{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};

        if (tokens.size() != 2) {
            std::string guide = "Usage: createFolder <folder_path>";
            this->sendMessage(clientSocket, guide.c_str());
            return;
        }

        const std::wstring folderPath = std::wstring(tokens[1].begin(), tokens[1].end());

        std::string result = winAPI.createFolder(folderPath.c_str());
        std::string response = (result.substr(0, 6) != "Failed") ? result : "Error: " + result;
        this->sendMessage(clientSocket, response.c_str());
    };

    handlers[COPY_FOLDER] = [this](SOCKET& clientSocket, const std::string& command) {
        std::istringstream iss(command);
        std::vector<std::string> tokens{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};

        if (tokens.size() != 3) {
            std::string guide = "Usage: copyFolder <source_folder> <destination_folder>";
            this->sendMessage(clientSocket, guide.c_str());
            return;
        }

        const std::wstring sourceFolder = std::wstring(tokens[1].begin(), tokens[1].end());
        const std::wstring destinationFolder = std::wstring(tokens[2].begin(), tokens[2].end());

        std::string result = winAPI.copyFolder(sourceFolder.c_str(), destinationFolder.c_str());
        this->sendMessage(clientSocket, result.c_str());
    };
    
    handlers[LIST_COMMANDS] = [this](SOCKET& clientSocket, const std::string& command) {
        std::string commands = "Available commands: \n\t\t\t\t";
        for (const auto& pair : messageMap)
            commands += pair.first + "\n\t\t\t\t";
        commands.pop_back();
        commands.pop_back();
        commands.pop_back();
        this->sendMessage(clientSocket, commands.c_str());
    };

    handlers[LIST_PROCESS] = [this](SOCKET& clientSocket, const std::string& command) {
        system("tasklist > output/process.txt");
        this->sendMessage(clientSocket, "Done!\n");
    };

    handlers[LIST_SERVICES] = [this](SOCKET& clientSocket, const std::string& command) {
        system("net start > output/services.txt");
        this->sendMessage(clientSocket, "Done!\n");

    };

}

void ServerSocket::handleEvent(SOCKET &clientSocket, const std::string& message) {
    MessageType messageType = hashMessage(message);
    auto it = handlers.find(messageType);
    if (it != handlers.end()) {
        it->second(clientSocket, message);
    } else {
        std::string invalidMessage = "INVALID MESSAGE";
        send(clientSocket, invalidMessage.c_str(), static_cast<int>(invalidMessage.length()), 0);
    }
}
