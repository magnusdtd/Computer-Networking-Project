#include "OAuthManager.hpp"

OAuthManager::OAuthManager(const std::string& oauthFilePath, const std::string& tokenFilePath, const std::string& scriptFilePath)
    : oauthFilePath(oauthFilePath), tokenFilePath(tokenFilePath), scriptFilePath(scriptFilePath), isStopThread(false) {
    readOAuthFile();
    getAccessToken();
    readAccessToken();
    startTokenRefreshThread();
}

OAuthManager::~OAuthManager() {
    stopTokenRefreshThread();
}

void OAuthManager::readOAuthFile() {
    std::fstream readBuffer(oauthFilePath, std::ios::in);
    if (!readBuffer.is_open()) {
        std::cerr << "Cannot open " + oauthFilePath << "\n";
        exit(2);
    }

    nlohmann::json jsonContent;
    readBuffer >> jsonContent;
    readBuffer.close();

    if (jsonContent.contains("installed") && jsonContent["installed"].contains("client_id") && jsonContent["installed"].contains("client_secret")) {
        clientId = jsonContent["installed"]["client_id"];
        clientSecret = jsonContent["installed"]["client_secret"];
    } else {
        std::cerr << "Invalid OAuth file format.\n";
        exit(3);
    }
}

void OAuthManager::getAccessToken() {
    std::fstream tokenFile(tokenFilePath, std::ios::in);
    if (!tokenFile.is_open()) {
        tokenFile.close();
        std::ofstream createFile(tokenFilePath);
        createFile.close();
        tokenFile.open(tokenFilePath, std::ios::in);
    }

    tokenFile.seekg(0, std::ios::end);
    if (tokenFile.tellg() == 0) {
        std::cerr << "Token file is empty or cannot be opened, running script to get new token." << '\n';
        std::string command = "powershell -ExecutionPolicy Bypass -File " 
                            + scriptFilePath + " "
                            + oauthFilePath + " "
                            + "./GmailAPI/account.json" + " "
                            + tokenFilePath;
        int result = system(command.c_str());
        if (result != 0) {
            std::cerr << "Failed to run script\n";
            tokenFile.close();
            exit(1);
        }
        tokenFile.close();
        tokenFile.open(tokenFilePath, std::ios::in);
        tokenFile.seekg(0, std::ios::end);
        if (!tokenFile.is_open() || tokenFile.tellg() == 0) {
            std::cerr << "Failed to obtain token after running script.ps1\n";
            tokenFile.close();
            exit(1);
        }

        std::fstream jsonFile(tokenFilePath, std::ios::in);

        nlohmann::json jsonResponse;
        jsonFile >> jsonResponse;

        auto now = std::chrono::system_clock::now();
        jsonResponse["obtained_at"] = std::to_string(std::chrono::system_clock::to_time_t(now));

        jsonFile.close();
        jsonFile.open(tokenFilePath, std::ios::trunc | std::ios::out);

        jsonFile.seekp(0, std::ios::beg); // Move the write position to the beginning
        jsonFile << jsonResponse << "\n";
        jsonFile.close();

    }
    tokenFile.close();
}

void OAuthManager::readAccessToken() {
    std::fstream tokenFile(tokenFilePath, std::ios::in);

    if (tokenFile.is_open()) {
        nlohmann::json tokenJson;
        tokenFile >> tokenJson;

        if (tokenJson.contains("access_token") && tokenJson.contains("refresh_token") && tokenJson.contains("token_type") && tokenJson.contains("obtained_at")) {

            accessToken = tokenJson["access_token"];
            refreshToken = tokenJson["refresh_token"];
            tokenType = tokenJson["token_type"];

            std::string obtainedAtStr = tokenJson["obtained_at"];
            auto obtainedAt = std::chrono::system_clock::from_time_t(static_cast<time_t>(std::stoll(obtainedAtStr)));
            auto expiresIn = std::chrono::seconds(tokenJson["expires_in"].get<int>());
            tokenExpirationTime = obtainedAt + expiresIn;
        } else {
            std::cerr << "Invalid token file format.\n";
            tokenFile.close();
            exit(1);
        }
    } else {
        std::cerr << "Unable to open " + tokenFilePath + " file\n";
        tokenFile.close();  
        exit(1);
    }
    tokenFile.close();
}

void OAuthManager::getRefreshToken() {
    std::cout << "Trying to refresh access token\n";

    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    int retryCount = 0;
    const int maxRetries = 3;

    while (retryCount < maxRetries) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            std::string postFields = "client_id=" + clientId + "&client_secret=" + clientSecret + "&refresh_token=" + refreshToken + "&grant_type=refresh_token";

            curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
                std::cerr << "Failed to get refresh token\n";
                retryCount++;
            } else {
                auto jsonResponse = nlohmann::json::parse(readBuffer);

                if (jsonResponse.contains("access_token") && jsonResponse.contains("expires_in")) {
                    std::fstream tokenFile(tokenFilePath, std::ios::in | std::ios::out);
                    if (tokenFile.is_open()) {
                        nlohmann::json tokenJson;
                        tokenFile >> tokenJson;

                        tokenJson["access_token"] = jsonResponse["access_token"];
                        tokenJson["expires_in"] = jsonResponse["expires_in"];

                        auto now = std::chrono::system_clock::now();
                        tokenJson["obtained_at"] = std::to_string(std::chrono::system_clock::to_time_t(now));

                        tokenFile.close();
                        tokenFile.open(tokenFilePath, std::ios::trunc | std::ios::out);
                        tokenFile << tokenJson << '\n';
                        tokenFile.close();

                        //std::cout << "Successfully refreshed access token\n";
                        curl_easy_cleanup(curl);
                        curl_global_cleanup();
                        return;
                    } else {
                        std::cerr << "Failed to open token file to write new access token\n";
                        retryCount++;
                    }
                } else {
                    std::cerr << "Failed to refresh access token: " << jsonResponse.dump() << '\n';
                    retryCount++;
                }
            }

            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();

        if (retryCount < maxRetries) {
            std::cerr << "Retrying to get refresh token (" << retryCount << "/" << maxRetries << ")\n";
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait before retrying
        }
    }

    std::cerr << "Exceeded maximum retries to get refresh token\n";
    exit(1);
}

void OAuthManager::startTokenRefreshThread() {
    {
        std::lock_guard<std::mutex> lock(stopMutex);
        isStopThread = false;
    }
    tokenRefreshThread = std::thread(&OAuthManager::refreshTokenLoop, this);
}

void OAuthManager::stopTokenRefreshThread() {
    {
        std::lock_guard<std::mutex> lock(stopMutex);
        isStopThread = true;
    }
    stopCondVar.notify_all();
    if (tokenRefreshThread.joinable())
        tokenRefreshThread.join();
}

void OAuthManager::refreshTokenLoop() {
    while (true) {
        {
            std::unique_lock<std::mutex> lock(stopMutex);
            if (stopCondVar.wait_for(lock, std::chrono::seconds(5), [this] { return isStopThread; })) {
                break;
            }
        }

        if (isTokenExpired()) {
            getRefreshToken();
            readAccessToken();
            std::cout << "New access token via refresh token!\n";
        }
    }
}

bool OAuthManager::isTokenExpired() const {
    return std::chrono::system_clock::now() >= tokenExpirationTime;
}

size_t OAuthManager::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}