#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
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
#include <zlib.h>

class GmailAPI {
private:
    std::string clientId;
    std::string clientSecret;


    std::string tokenType;
        
    std::string oauthFileName;
    std::string tokenFileName;
    std::string scriptFileName;
    std::string messageListFileName;


    std::atomic<bool> stopThread;
    std::thread tokenRefreshThread;

    void readOAuthFile();

    void getAccessToken();

    void readAccessToken();

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

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

    // For testing
    std::string accessToken;
    std::string refreshToken;

    time_t tokenExpirationTime;

    GmailAPI(const std::string oauthFileName, const std::string tokenFileName, const std::string scriptFileName, const std::string messageListFileName);

    void send(const std::string& to, const std::string& subject, const std::string& body);
    
    void send(const std::string& to, const std::string& subject, const std::string& body, const std::string &filePath);

    void query(const std::string& query, const std::string& userName, const std::string& outputFile);

    void markAsRead();

    std::vector<std::string> extractMessageIds(const std::string& filename);

    void startTokenRefreshThread() {
        stopThread = false;
        tokenRefreshThread = std::thread(&GmailAPI::tokenRefreshLoop, this);
    }
    
    void stopTokenRefreshThread() {
        std:: cout << "Trying to stop thread\n";
        stopThread = true;
        if (tokenRefreshThread.joinable())
            tokenRefreshThread.join();
        std::cout << "Has stopped thread\n";    
    }
};