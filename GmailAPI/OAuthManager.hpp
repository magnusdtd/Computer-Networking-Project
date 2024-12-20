#ifndef OAUTH_MANAGER_HPP
#define OAUTH_MANAGER_HPP

#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <fstream>
#include <stdexcept>

class OAuthManager {
public:
    OAuthManager(const std::string& oauthFilePath, const std::string& tokenFilePath, const std::string& scriptFilePath);
    ~OAuthManager();

    void startTokenRefreshThread();
    void stopTokenRefreshThread();

protected:
    std::string clientId;
    std::string clientSecret;
    std::string oauthFilePath;
    std::string tokenFilePath;
    std::string refreshTokenFilePath;
    std::string scriptFilePath;
    std::string accessToken;
    std::string refreshToken;
    std::string tokenType;
    std::chrono::system_clock::time_point tokenExpirationTime;
    std::mutex stopMutex;
    std::condition_variable stopCondVar;
    bool isStopThread;
    std::thread tokenRefreshThread;

    void readOAuthFile();

    void getAccessToken();

    void readAccessToken();

    void getRefreshToken();

    void refreshTokenLoop();

    bool isTokenExpired() const;

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

};

#endif