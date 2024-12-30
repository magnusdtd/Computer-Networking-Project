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

std::vector<std::string> ClientSocket::splitArguments(const std::string &str)
{
    std::vector<std::string> result;
    std::regex re(R"((\"[^\"]*\")|(\S+))");
    std::sregex_iterator it(str.begin(), str.end(), re);
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

ClientSocket::ClientSocket(const std::string &oauthFilePath, const std::string &tokenFilePath, const std::string &scriptFilePath)
    : GmailAPI(oauthFilePath, tokenFilePath, scriptFilePath), bytesReceived(0), stopClient(false), isStopMQThread(false)
{
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

    // std::string serverIp = discoverServer();
    // if (serverIp.empty()) {
    //     std::cerr << "Failed to discover server.\n";
    //     closesocket(clientSocket);
    //     WSACleanup();
    //     exit(1);
    // }

    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(PORT);
    InetPton(AF_INET, _T(SERVER_IP), &clientAddress.sin_addr.s_addr);
    // InetPton(AF_INET, std::wstring(serverIp.begin(), serverIp.end()).c_str(), &clientAddress.sin_addr.s_addr);

    if (connect(clientSocket, (SOCKADDR*)&clientAddress, sizeof(clientAddress)) == SOCKET_ERROR) {
        std::cerr << "Connection to server failed.\n";
        closesocket(clientSocket);
        WSACleanup();
        exit(1);
    }

    messageQueueThread = std::thread(&ClientSocket::processQueue, this);
}

std::string ClientSocket::discoverServer() {
    SOCKET discoverySocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (discoverySocket == INVALID_SOCKET) {
        std::cerr << "Discovery socket creation failed: " << WSAGetLastError() << '\n';
        return "";
    }

    sockaddr_in broadcastAddress;
    broadcastAddress.sin_family = AF_INET;
    broadcastAddress.sin_addr.s_addr = INADDR_BROADCAST;
    broadcastAddress.sin_port = htons(DISCOVERY_PORT);

    char broadcast = 1;
    if (setsockopt(discoverySocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) == SOCKET_ERROR) {
        std::cerr << "setsockopt failed: " << WSAGetLastError() << '\n';
        closesocket(discoverySocket);
        return "";
    }

    if (sendto(discoverySocket, DISCOVERY_MESSAGE, strlen(DISCOVERY_MESSAGE), 0, (SOCKADDR*)&broadcastAddress, sizeof(broadcastAddress)) == SOCKET_ERROR) {
        std::cerr << "Discovery sendto failed: " << WSAGetLastError() << '\n';
        closesocket(discoverySocket);
        return "";
    }

    char buffer[1024];
    sockaddr_in serverAddress;
    int serverAddressSize = sizeof(serverAddress);

    int bytesReceived = recvfrom(discoverySocket, buffer, sizeof(buffer) - 1, 0, (SOCKADDR*)&serverAddress, &serverAddressSize);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Discovery recvfrom failed: " << WSAGetLastError() << '\n';
        closesocket(discoverySocket);
        return "";
    }

    buffer[bytesReceived] = '\0';
    closesocket(discoverySocket);
    return std::string(buffer);
}

ClientSocket::~ClientSocket() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        isStopMQThread = true;
    }
    while (!userQueue.empty()) { 
        User *temp = userQueue.front();
        userQueue.pop(); 
        delete temp;
        temp = nullptr;
    }
    queueCondVar.notify_all();
    messageQueueThread.join();
    closesocket(clientSocket);
    WSACleanup();
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

bool ClientSocket::executeCommand(std::string &response, std::string& receivedFilePath, const std::string &command, const std::string& arg1, const std::string& arg2)
{
    std::string fullCommand;
    if (!arg1.empty() && !arg2.empty())
        fullCommand = command + " " + arg1 + " " + arg2;
    else if (!arg1.empty() && arg2.empty())
        fullCommand = command + " " + arg1;
    else if (arg1.empty() && arg2.empty())
        fullCommand = command;
    else {
        response = "Invalid command!";
        return false;
    }

    ::send(clientSocket, fullCommand.c_str(), static_cast<int>(fullCommand.length()), 0);

    if (command == "STOP") {
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        response = "Server has been stop!";
        return true;
    } else if (command == "shutdown") {
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        response = "Server has been shutdown!";
        std::cout << "Failed to connect to server.\n";
        return true;
    } else if (command == "restart") {
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        response = "Server has been restart!";
        std::cout << "Failed to connect to server.\n";
        return true;
    }
    else if (   command == "listProcess" ||
                command == "listService" ||
                command == "listRunningApp" ||
                command == "listInstalledApp" ||
                command == "listFiles" ||
                command == "captureScreen" ||
                command == "disableKeylogger" ||
                command == "screenRecording" ) {
        // These command has file send back to client
        // Receive the file name from the server
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "\t-> [Response from server] " << buffer << '\n';

            // Extract the file path from the server's response command
            std::string temp(buffer); 
            response = temp;
            std::string keyword = " at ";
            size_t pos = temp.find(keyword);
            std::string filePath = temp.substr(pos + keyword.length());

            // Extract the file name from the file path
            std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
            std::string outputFilePath = "./output-client/" + fileName; 
            receivedFilePath = outputFilePath;
            receiveFile(outputFilePath);

            return true;
        } else {
            std::cerr << "\t-> [Client] Failed to receive message from the server.\n";
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
            std::cerr << "\t-> [Client] Failed to receive message from the server.\n";
            return false;
        }
    }
}

void ClientSocket::processQueue() {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCondVar.wait(lock, [this] { return !userQueue.empty() || isStopMQThread; });

        if (isStopMQThread && userQueue.empty())
            break;

        std::vector<User*> batch;
        while (!userQueue.empty()) {
            batch.push_back(userQueue.front());
            userQueue.pop();
        }
        lock.unlock();

        for (User* user : batch) {
            std::cout << "Processing user: " << user->name << "\n";

            // Combine subject and body for command search
            std::string combinedText = user->subject + " " + user->body;
            bool isValidCommand = false;

            for (const auto& message : messageMap) {
                const std::string& command = message.first;
                const std::string pattern = "👻 " + command;
                size_t position = combinedText.find(pattern);

                if (position != std::string::npos) {
                    isValidCommand = true;
                    std::string argsStr = combinedText.substr(position + pattern.length());
                    std::vector<std::string> args = splitArguments(argsStr);

                    std::string response, filePath;
                    bool success = false;

                    if ((command == "copy" || command == "copyFolder") && args.size() >= 2) {
                        success = executeCommand(response, filePath, command, "\"" + args[0] + "\"", "\"" + args[1] + "\"");
                    } else if ((command == "delete" || command == "createFolder" || command == "startApp" || command == "terminateProcess" || command == "listFiles" || command == "screenRecording") && args.size() >= 1) {
                        success = executeCommand(response, filePath, command, "\"" + args[0] + "\"");
                    } else {
                        success = executeCommand(response, filePath, command);
                    }

                    std::cout << "Command: " << command << "\n";

                    // Send the result back to the user via email
                    if (success) {
                        if (filePath.empty())
                            send(user->email, command + " worked as expected", response);
                        else
                            send(user->email, command + " worked as expected", response, filePath);
                    } else {
                        send(user->email, command + " didn't work as expected", "There was some mysterious error or something not work as expected in the system, try to reconnect to server and build/run server again.");
                    }
                }
            }

            if (!isValidCommand)
                std::cout << "No command found\n";
            
            markAsRead(user->messageId);

            delete user;
        }
    }
}

void ClientSocket::fetchMessageDetails(CURL *curl, const std::string &messageUrl, std::string &readBuffer)
{
    curl_easy_setopt(curl, CURLOPT_URL, messageUrl.c_str());
    readBuffer.clear();
    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        auto messageResponse = nlohmann::json::parse(readBuffer);
        if (messageResponse.contains("payload")) {
            const auto& payload = messageResponse["payload"];
            std::string subject;
            std::string body;
            std::string senderName;
            std::string senderEmail;
            std::string messageId;

            // Extract message ID
            if (messageResponse.contains("id")) {
                messageId = messageResponse["id"];
            }

            // Extract subject, sender, and sender email
            if (payload.contains("headers")) {
                for (const auto& header : payload["headers"]) {
                    if (header["name"] == "Subject") {
                        subject = header["value"];
                    } else if (header["name"] == "From") {
                        senderName = header["value"];
                        size_t start = senderName.find('<');
                        size_t end = senderName.find('>');
                        if (start != std::string::npos && end != std::string::npos) {
                            senderEmail = senderName.substr(start + 1, end - start - 1);
                        }
                    }
                }
            }

            // Extract body
            if (payload.contains("mimeType") && payload["mimeType"] == "multipart/alternative" && payload.contains("parts")) {
                for (const auto& part : payload["parts"]) {
                    if (part.contains("mimeType") && part["mimeType"] == "text/plain" && part.contains("body") && part["body"].contains("data")) {
                        std::string temp = part["body"]["data"];
                        body = base64->decode(temp);
                    }
                }
            } else if (payload.contains("body") && payload["body"].contains("data")) {
                body = base64->decode(payload["body"]["data"]);
            }

            // Create a User object and add it to the queue
            User* user = new User(messageId, senderName, senderEmail, subject, body);
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                userQueue.push(user);
            }
            queueCondVar.notify_one();
        }
    } else {
        std::cerr << "Failed to fetch message details for URL: " << messageUrl << "\n";
    }
}