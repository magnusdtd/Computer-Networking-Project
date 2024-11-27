#ifndef GMAIL_API_HPP
#define GMAIL_API_HPP

#include "OAuthManager.hpp"
#include "Base64.hpp"
#include "User.hpp"
#include "./../Client/ClientSocket.hpp"
#include <queue>
#include <sstream>

class GmailAPI : OAuthManager {
public:
    GmailAPI(const std::string& oauthFilePath, const std::string& tokenFilePath, const std::string& scriptFilePath, const std::string& messageListFilePath, ClientSocket& clientSocket);

    ~GmailAPI();

    void send(const std::string& to, const std::string& subject, const std::string& body);

    void send(const std::string& to, const std::string& subject, const std::string& body, const std::string& filePath);

    void query(const std::string& query, const std::string& userName);

    void markAsRead();

private:
    std::string messageListFilePath;
    Base64 *base64;
    ClientSocket& clientSocket;

    std::queue<User*> userQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondVar;
    std::thread messageQueueThread;
    bool isStopMQThread;

    std::string readFile(const std::string& filePath);

    std::vector<unsigned char> readBinaryFile(const std::string& filePath);

    CURL* initializeCurl(const std::string& url, const std::string& tokenType, const std::string& accessToken, std::string& readBuffer);

    void fetchMessageDetails(CURL* curl, const std::string& messageUrl, std::string& readBuffer, std::ofstream& file);

    void sendFile(const std::string& url, const std::string& emailData, struct curl_slist* headers);

    std::vector<std::string> extractMessageIds(const std::string& filename);

    void processQueue();
};

#endif