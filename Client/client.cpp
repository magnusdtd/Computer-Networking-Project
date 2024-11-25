#include "ClientSocket.hpp"
#include "./../GmailAPI/GmailAPI.hpp"

void checkEmails(GmailAPI& gmail, ClientSocket& client) {
    while (true) {
        gmail.query("is:unread", "");
        if (gmail.searchPattern("STOP")) {
            std::string stopMessage = "STOP";
            send(client.getSocket(), stopMessage.c_str(), static_cast<int>(stopMessage.length()), 0);
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

int main() {

    GmailAPI gmail(
        "./GmailAPI/oauth2.json", 
        "./GmailAPI/token.json", 
        "./GmailAPI/script-auto.ps1", 
        "./GmailAPI/message-list.txt"
    );

    gmail.startTokenRefreshThread();

    ClientSocket client;

    // Start the email checking thread
    std::thread emailThread(checkEmails, std::ref(gmail), std::ref(client));

    try {
        char buffer[1024] = {0};
        int bytesReceived = 0;

        while (true) {
            std::string userInput;
            std::cout << "Enter message to send to server: ";
            std::getline(std::cin, userInput);

            if (userInput.empty()) {
                continue;
            } else if (userInput == "STOP") {
                break;
            }

            send(client.getSocket(), userInput.c_str(), static_cast<int>(userInput.length()), 0);

            if (userInput.find("listProcess") == 0 ||
                userInput.find("listService") == 0 ||
                userInput.find("listRunningApp") == 0 ||
                userInput.find("listInstalledApp") == 0 ||
                userInput.find("listFiles") == 0 ||
                userInput.find("captureScreen") == 0 ||
                userInput.find("disableKeylogger") == 0 
                ) {
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
                    if (pos != std::string::npos) {
                        filePath = response.substr(pos + keyword.length());
                    } else
                        continue;

                    // Extract the file name from the file path
                    std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
                    std::string outputFilePath = "./output-client/" + fileName;
                    client.receiveFile(outputFilePath);
                } else {
                    std::cerr << "\t-> [Failed to receive file name from the server]\n";
                }
            } else {
                memset(buffer, 0, sizeof(buffer));
                bytesReceived = recv(client.getSocket(), buffer, sizeof(buffer) - 1, 0);
                if (bytesReceived > 0) {
                    buffer[bytesReceived] = '\0';
                    std::cout << "\t-> [Response from server] " << buffer << '\n';
                } else {
                    std::cerr << "\t-> [Failed to receive response from the server]\n";
                }
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
        return 1;
    }

   gmail.stopTokenRefreshThread();

    // Join the email checking thread
    if (emailThread.joinable()) {
        emailThread.join();
    }

    return 0;
}