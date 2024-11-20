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
        {"listService", LIST_SERVICES},
        {"startApp", START_APP},
        {"terminateProcess", TERMINATE_PROCESS},
        {"listRunningApp", LIST_RUNNING_APP},
        {"listInstalledApp", LIST_INSTALLED_APP},
        {"listFiles", LIST_FILES},
    };
}

MessageType ServerSocket::hashMessage(const std::string message) {
    std::istringstream iss(message);
    std::string command;
    iss >> command;

    auto it = messageMap.find(command);
    return (it != messageMap.end() ? it->second : INVALID);
}

std::vector<std::string> ServerSocket::parseCommand(const std::string& command) {
    std::istringstream iss(command);
    return std::vector<std::string>{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
}

void ServerSocket::sendResponse(SOCKET &clientSocket, const std::string& response) {
    this->sendMessage(clientSocket, response.c_str());
}

void ServerSocket::initializeHandlers() {
    handlers[SHUTDOWN] = [this](SOCKET &clientSocket, const std::string& command) { 
        sendResponse(clientSocket, "Trying to shutdown server!\n");
        winAPI.systemShutdown(); 
    };

    handlers[RESTART] = [this](SOCKET &clientSocket, const std::string& command) { 
        sendResponse(clientSocket, "Trying to restart server!\n");
        LPWSTR restartMessage = L"RESTART"; 
        winAPI.systemRestart(restartMessage); 
    };

    handlers[GET_IP] = [this](SOCKET &clientSocket, const std::string& command) { 
        this->sendIPAddress(clientSocket); 
    };

    handlers[HELLO_SERVER] = [this](SOCKET &clientSocket, const std::string& command) {
        sendResponse(clientSocket, "Hello from server!\n");
    };

    handlers[STOP] = [this](SOCKET &clientSocket, const std::string& command) {
        this->~ServerSocket();
        closesocket(clientSocket);
        exit(0);
    };

    handlers[CAPTURE_SCREEN] = [this](SOCKET &clientSocket, const std::string& command) {
        std::string result = winAPI.saveScreenshot();
        sendResponse(clientSocket, (result.substr(0, 6) != "Failed") ? "Capture screen successful, new file: " + result : "Error: " + result);
    };

    handlers[COPY_FILE] = [this](SOCKET &clientSocket, const std::string& command) {
        auto tokens = parseCommand(command);
        if (tokens.size() != 3) {
            sendResponse(clientSocket, "Usage: copy <source_path> <destination_path>");
            return;
        }

        const std::wstring source = std::wstring(tokens[1].begin(), tokens[1].end());
        const std::wstring destination = std::wstring(tokens[2].begin(), tokens[2].end());
        std::string result = winAPI.copyFile(source.c_str(), destination.c_str());
        sendResponse(clientSocket, (result.substr(0, 6) != "Failed") ? result : "Error: " + result);
    };

    handlers[DELETE_FILE] = [this](SOCKET &clientSocket, const std::string& command) { 
        auto tokens = parseCommand(command);
        if (tokens.size() != 2) {
            sendResponse(clientSocket, "Usage: delete <source_path>");
            return;
        }

        const std::wstring source = std::wstring(tokens[1].begin(), tokens[1].end());
        std::string result = winAPI.deleteFile(source.c_str());
        sendResponse(clientSocket, (result.substr(0, 6) != "Failed") ? result : "Error: " + result);
    };

    handlers[CREATE_FOLDER] = [this](SOCKET &clientSocket, const std::string& command) {
        auto tokens = parseCommand(command);
        if (tokens.size() != 2) {
            sendResponse(clientSocket, "Usage: createFolder <folder_path>");
            return;
        }

        const std::wstring folderPath = std::wstring(tokens[1].begin(), tokens[1].end());
        std::string result = winAPI.createFolder(folderPath.c_str());
        sendResponse(clientSocket, (result.substr(0, 6) != "Failed") ? result : "Error: " + result);
    };

    handlers[COPY_FOLDER] = [this](SOCKET &clientSocket, const std::string& command) {
        auto tokens = parseCommand(command);
        if (tokens.size() != 3) {
            sendResponse(clientSocket, "Usage: copyFolder <source_folder> <destination_folder>");
            return;
        }

        const std::wstring sourceFolder = std::wstring(tokens[1].begin(), tokens[1].end());
        const std::wstring destinationFolder = std::wstring(tokens[2].begin(), tokens[2].end());
        std::string result = winAPI.copyFolder(sourceFolder.c_str(), destinationFolder.c_str());
        sendResponse(clientSocket, (result.substr(0, 6) != "Failed") ? result : "Error: " + result);
    };
    
    handlers[LIST_COMMANDS] = [this](SOCKET &clientSocket, const std::string& command) {
        std::string commands = "Available commands: \n\t\t\t\t";
        for (const auto& pair : messageMap)
            commands += pair.first + "\n\t\t\t\t";
        commands.pop_back();
        commands.pop_back();
        commands.pop_back();
        sendResponse(clientSocket, commands);
    };

    handlers[LIST_PROCESS] = [this](SOCKET &clientSocket, const std::string& command) {
        std::string fileName = winAPI.generateName("process_list", "txt");
        std::string filePath = "./output/" + fileName;
        std::string systemCommand = "tasklist > " + filePath;
        int result = system(systemCommand.c_str());

        sendResponse(clientSocket, (result != 0) ? "Error: Command execution failed with exit code: " + std::to_string(result) : "Successfully list all processes at " + filePath);
    };

    handlers[LIST_SERVICES] = [this](SOCKET &clientSocket, const std::string& command) {
        std::string fileName = winAPI.generateName("services_list", "txt");
        std::string filePath = "./output/" + fileName;
        std::string systemCommand = "net start > " + filePath;
        int result = system(systemCommand.c_str());

        sendResponse(clientSocket, (result != 0) ? "Error: Command execution failed with exit code: " + std::to_string(result) : "Successfully list all services at " + filePath);
    };

    handlers[START_APP] = [this](SOCKET &clientSocket, const std::string& command) {
        auto tokens = parseCommand(command);
        if (tokens.size() < 2) {
            sendResponse(clientSocket, "Usage: startApp <application_path>");
            return;
        }

        std::wstring applicationPath = std::wstring(command.begin() + command.find(' ') + 1, command.end());
        std::string result = winAPI.StartApplication(applicationPath);
        sendResponse(clientSocket, result);
    };

    handlers[TERMINATE_PROCESS] = [this](SOCKET &clientSocket, const std::string& command) {
        auto tokens = parseCommand(command);
        if (tokens.size() != 2) {
            sendResponse(clientSocket, "Usage: terminateProcess <process_id>");
            return;
        }

        DWORD processID = std::stoul(tokens[1]);
        std::string result = winAPI.TerminateProcessByID(processID);
        sendResponse(clientSocket, result);
    };

    handlers[LIST_INSTALLED_APP] = [this](SOCKET &clientSocket, const std::string& command) {
        std::string fileName = winAPI.generateName("installed_app_list", "txt");
        std::string filePath = "./output/" + fileName;
        std::string result = winAPI.listInstalledApp(filePath);
        sendResponse(clientSocket, (result.substr(0, 6) != "Failed") ? result : "Error: " + result);
    };

    handlers[LIST_RUNNING_APP] = [this](SOCKET &clientSocket, const std::string &command) {
        std::string fileName = winAPI.generateName("running_app_list", "txt");
        std::string filePath = "./output/" + fileName;
        std::string result = winAPI.listRunningApp(filePath);
        sendResponse(clientSocket, (result.substr(0, 6) != "Failed") ? result : "Error: " + result);
    };

    handlers[LIST_FILES] = [this](SOCKET &clientSocket, const std::string& command) {
        auto tokens = parseCommand(command);
        if (tokens.size() < 2) {
            sendResponse(clientSocket, "Usage: listFiles <directory_path>");
            return;
        }
        const std::wstring directoryPath = std::wstring(command.begin() + command.find(' ') + 1, command.end());
        std::string fileName = winAPI.generateName("file_list", "txt");
        std::string filePath = "./output/" + fileName;
        std::string result = winAPI.listFilesInDirectory(directoryPath, filePath);
        sendResponse(clientSocket, (result.substr(0, 6) != "Failed") ? result : "Error: " + result);
    };
}

void ServerSocket::handleEvent(SOCKET &clientSocket, const std::string& message) {
    MessageType messageType = hashMessage(message);
    auto it = handlers.find(messageType);
    if (it != handlers.end()) {
        it->second(clientSocket, message);
    } else {
        sendResponse(clientSocket, "INVALID MESSAGE");
    }
}
