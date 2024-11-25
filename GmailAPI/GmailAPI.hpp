#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <future>
#include <atomic>

#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

class GmailAPI {
private:
    std::string clientId;
    std::string clientSecret;

    const std::string accountFilePath = "./GmailAPI/account.json";

    std::string tokenType;
        
    std::string oauthFilePath;
    std::string tokenFilePath;
    std::string scriptFilePath;
    std::string messageListFilePath;

    std::string accessToken;
    std::string refreshToken;

    time_t tokenExpirationTime;


    std::atomic<bool> stopThread;
    std::thread tokenRefreshThread;

    void readOAuthFile();

    void getAccessToken();

    void readAccessToken();

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    std::string base64Encode(const std::string &input);

    std::string base64EncodeChunk(const std::vector<unsigned char>& input);

    std::string base64Encode(const std::vector<unsigned char> &input);

    std::string base64Decode(const std::string &input);

    std::string readFile(const std::string &filePath);

    std::vector<unsigned char> readBinaryFile(const std::string &filePath);

    CURL* initializeCurl(const std::string& url, const std::string& tokenType, const std::string& accessToken, std::string& readBuffer);

    void fetchMessageDetails(CURL* curl, const std::string& messageUrl, std::string& readBuffer, std::ofstream& file);

    void tokenRefreshLoop();

    bool isTokenExpired() { return std::time(nullptr) >= tokenExpirationTime; }

    void refreshAccessToken();

    void sendFile(const std::string &url, const std::string &emailData, struct curl_slist *headers);

public:

    GmailAPI(const std::string oauthFilePath, const std::string tokenFilePath, const std::string scriptFilePath, const std::string messageListFilePath);

    void send(const std::string& to, const std::string& subject, const std::string& body);
    
    void send(const std::string& to, const std::string& subject, const std::string& body, const std::string &filePath);

    void query(const std::string& query, const std::string& userName);

    void markAsRead();

    std::vector<std::string> extractMessageIds(const std::string& filename);

    void startTokenRefreshThread();
    
    void stopTokenRefreshThread();

    bool searchPattern(const std::string& pattern);

};