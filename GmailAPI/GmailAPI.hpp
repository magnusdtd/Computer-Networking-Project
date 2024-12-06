#ifndef GMAIL_API_HPP
#define GMAIL_API_HPP

#include "OAuthManager.hpp"
#include "Base64.hpp"
#include "User.hpp"
#include <queue>
#include <sstream>

class GmailAPI : OAuthManager {
public:
    GmailAPI(const std::string& oauthFilePath, const std::string& tokenFilePath, const std::string& scriptFilePath);

    ~GmailAPI();

    void send(const std::string& to, const std::string& subject, const std::string& body);

    void send(const std::string& to, const std::string& subject, const std::string& body, const std::string& filePath);

    void query(const std::string& query, const std::string& userName);

    void markAsRead(const std::string& messageId);

protected:

    Base64 *base64;

private:

    std::string readFile(const std::string& filePath);

    std::vector<unsigned char> readBinaryFile(const std::string& filePath);

    CURL* initializeCurl(const std::string& url, const std::string& tokenType, const std::string& accessToken, std::string& readBuffer);

    virtual void fetchMessageDetails(CURL* curl, const std::string& messageUrl, std::string& readBuffer) = 0;
};

#endif