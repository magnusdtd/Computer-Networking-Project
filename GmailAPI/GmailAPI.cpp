#include "GmailAPI.hpp"

void GmailAPI::readOAuthFile() {
    std::fstream readBuffer(oauthFileName, std::ios::in);
    if (!readBuffer.is_open()) {
        throw std::runtime_error("Cannot open " + oauthFileName);
    }

    nlohmann::json jsonContent;
    readBuffer >> jsonContent;
    readBuffer.close();

    if (jsonContent.contains("installed") && jsonContent["installed"].contains("client_id") && jsonContent["installed"].contains("client_secret")) {
        clientId = jsonContent["installed"]["client_id"];
        clientSecret = jsonContent["installed"]["client_secret"];
    } else {
        throw std::runtime_error("Invalid OAuth file format.");
    }
}

void GmailAPI::getAccessToken() {
    std::fstream tokenFile(tokenFileName, std::ios::in);
    // Check is file is empty or can't open, use ostream to automatically create new file if it does exsist.

    if (!tokenFile.is_open()) {
        tokenFile.close();
        std::ofstream createFile(tokenFileName);
        createFile.close();
        tokenFile.open(tokenFileName, std::ios::in);
    }

    if (tokenFile.peek() == std::ifstream::traits_type::eof()) {
        std::cerr << "Token file is empty or cannot be opened, running script to get new token." << '\n';
        int result = system(("powershell -ExecutionPolicy Bypass -File .\\" + scriptFileName).c_str());
        if (result != 0) {
            std::cerr << "Failed to run script\n";
            tokenFile.close();
            exit(1);
        }
        // Reopen the file after running the script
        tokenFile.close();
        tokenFile.open(tokenFileName, std::ios::in);
        if (!tokenFile.is_open() || tokenFile.peek() == std::ifstream::traits_type::eof()) {
            std::cerr << "Failed to obtain token after running script.ps1\n";
            tokenFile.close();
            exit(1);
        }
    }
    tokenFile.close();
}

void GmailAPI::readAccessToken() {
    std::fstream tokenFile(tokenFileName, std::ios::in);
    if (tokenFile.is_open()) {
        nlohmann::json tokenJson;
        tokenFile >> tokenJson;
        if (tokenJson.contains("access_token") && tokenJson.contains("refresh_token") && tokenJson.contains("token_type")) {
            accessToken = tokenJson["access_token"];
            refreshToken = tokenJson["refresh_token"];
            tokenType = tokenJson["token_type"];
            tokenExpirationTime = std::time(nullptr) + tokenJson["expires_in"];
        } else {
            std::cerr << "Invalid token file format.\n";
            tokenFile.close();
            exit(1);
        }
    } else {
        std::cerr << "Unable to open " + tokenFileName + " file\n";
        tokenFile.close();  
        exit(1);
    }
    tokenFile.close();
}

std::string GmailAPI::base64Encode(const std::string &input) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input.c_str(), static_cast<int>(input.length()));
    BIO_flush(bio);

    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string encodedData(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return encodedData;
}

std::string GmailAPI::base64EncodeChunk(const std::vector<unsigned char>& input) {
    std::string encoded;
    // Use OpenSSL's EVP_ENCODE_BLOCK function for efficient Base64 encoding
    encoded.resize(4 * ((input.size() + 2) / 3)); // Resize to accommodate base64 output
    int encodedLength = EVP_EncodeBlock((unsigned char*)encoded.data(), (const unsigned char*)input.data(), static_cast<int>(input.size()));
    encoded.resize(encodedLength); // Resize to actual encoded length
    return encoded;
}

std::string GmailAPI::base64Encode(const std::vector<unsigned char>& input) {
    // Determine chunk size based on desired threadS count and file size
    size_t chunkSize = input.size() / std::thread::hardware_concurrency();

    std::vector<std::future<std::string>> futures;
    std::string encoded;

    // Create threads to process chunks concurrently
    for (size_t i = 0; i < input.size(); i += chunkSize) {
        size_t end = i + chunkSize >  input.size() ? input.size() : i + chunkSize;
        std::vector<unsigned char> chunk(input.begin() + i, input.begin() + end);

        futures.push_back(std::async(std::launch::async, [this, chunk]() { return base64EncodeChunk(chunk); }));
    }

    // Concatenate results from each thread
    for (auto& future : futures)
        encoded += future.get();

    return encoded;
}

std::string GmailAPI::base64Decode(const std::string &input)
{
    std::string out;

    int decodeLen = static_cast<int>(input.size());
    std::vector<char> buffer(decodeLen);

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bio = BIO_new_mem_buf(input.data(), decodeLen);
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    int decoded_size = BIO_read(bio, buffer.data(), decodeLen);
    out.assign(buffer.data(), decoded_size);

    BIO_free_all(bio);

    return out;
}

std::string GmailAPI::readFile(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file)
        throw std::runtime_error("Unable to open file: " + filePath);
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

std::vector<unsigned char> GmailAPI::readBinaryFile(const std::string &filePath) {
    // Function to read a binary file into a vector of unsigned char
    std::ifstream file(filePath, std::ios::binary);
    if (!file )throw std::runtime_error("Unable to open file: " + filePath);

    // Read the file into a vector
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return buffer;
}

void GmailAPI::tokenRefreshLoop() {
    while (!stopThread) {
        if (isTokenExpired())
            refreshAccessToken();

        std::this_thread::sleep_for(std::chrono::seconds(10));

        if (stopThread) break;
    }
}

void GmailAPI::refreshAccessToken() {
    std::cout << "Trying to get refresh token\n";

    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        std::string postFields = "client_id=" + clientId + "&client_secret=" + clientSecret + "&refresh_token=" + refreshToken + "&grant_type=refresh_token";

        curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    // Parse the JSON response
    auto jsonResponse = nlohmann::json::parse(readBuffer);

    if (jsonResponse.contains("access_token") && jsonResponse.contains("expires_in")) {
        tokenExpirationTime = std::time(nullptr) + jsonResponse["expires_in"].get<int>();

        std::string newAccessToken = jsonResponse["access_token"];
        if (!newAccessToken.empty()) {
            accessToken = newAccessToken;
            std::ofstream tokenFile("new_access_token.txt", std::ios::out);
            if (tokenFile.is_open()) {
                tokenFile << "New Access Token: " << newAccessToken << "\n";
                tokenFile.close();
            } else
                std::cerr << "Failed to open file to write new access token\n";
        } else {
            std::cerr << "Failed to refresh access token\n";
        }
    } else {
        std::cerr << "Failed to refresh access token: " << jsonResponse.dump() << '\n';
    }
}

GmailAPI::GmailAPI(
    const std::string oauthFileName, 
    const std::string tokenFileName, 
    const std::string scriptFileName, 
    const std::string messageListFileName) : 
    oauthFileName(oauthFileName), 
    tokenFileName(tokenFileName), 
    scriptFileName(scriptFileName),
    messageListFileName(messageListFileName)
{
    readOAuthFile();
    getAccessToken();
    readAccessToken();
}

void GmailAPI::send(const std::string &to, const std::string &subject, const std::string &body) {
    std::string url = "https://www.googleapis.com/gmail/v1/users/me/messages/send";
    std::string readBuffer;

    CURL* curl = initializeCurl(url, tokenType, accessToken, readBuffer);
    if (!curl) {
        std::cerr << "Failed to initialize CURL\n";
        return;
    }

    struct curl_slist* headers = nullptr;
    std::string bearerToken = "Authorization: " + tokenType + " " + accessToken;
    headers = curl_slist_append(headers, bearerToken.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    nlohmann::json emailJson;
    std::string rawEmail = "To: " + to + "\r\n" + "Subject: " + subject + "\r\n\r\n" + body;
    std::string encodedEmail = base64Encode(rawEmail);
    emailJson["raw"] = encodedEmail;
    std::string emailData = emailJson.dump();

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, emailData.c_str());

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
    } else {
        std::cout << "Email sent successfully!\n";
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();
}

void GmailAPI::sendFile(const std::string &url, const std::string &emailData, struct curl_slist *headers) {
    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, emailData.c_str());
        curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L); // Increase buffer size
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0); // Enable HTTP/2

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
        } else {
            std::cout << "Email sent successfully!\n";
        }

        curl_easy_cleanup(curl);
    }
}

void GmailAPI::send(const std::string &to, const std::string &subject, const std::string &body, const std::string &filePath) {
    std::string url = "https://www.googleapis.com/gmail/v1/users/me/messages/send";
    std::string readBuffer;

    struct curl_slist* headers = nullptr;
    std::string bearerToken = "Authorization: " + tokenType + " " + accessToken;
    headers = curl_slist_append(headers, bearerToken.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Determine the MIME type based on the file extension
    std::string mimeType, encodedFile;
    std::string fileExtension = filePath.substr(filePath.find_last_of(".") + 1);
    
    if (fileExtension == "jpg" || fileExtension == "png") {
        mimeType = (fileExtension == "jpg") ? "image/jpeg" : "image/png";
        std::vector<unsigned char> fileContent = readBinaryFile(filePath);
        encodedFile = base64Encode(fileContent);
    }
    else if (fileExtension == "txt") {
        mimeType = "text/plain";
        std::string fileContent = readFile(filePath);
        encodedFile = base64Encode(fileContent);
    }
    else if (fileExtension == "mp4" || fileExtension == "avi" || fileExtension == "mov") {
        // Set the MIME type for the video file
        if (fileExtension == "mp4")
            mimeType = "video/mp4";
        else if (fileExtension == "avi")
            mimeType = "video/x-msvideo";
        else if (fileExtension == "mov")
            mimeType = "video/quicktime";

        // Read the binary content of the video file
        std::vector<unsigned char> fileContent = readBinaryFile(filePath);

        // Encode the video data to base64
        encodedFile = base64Encode(fileContent);
    }
    else {
        std::cerr << "Unsupported file type\n";
        return;
    }

    std::fstream out("encodedData.txt", std::ios::out);
    out << encodedFile;
    out.close();

    // Construct the email with attachment
    std::string boundary = "boundary";
    std::string rawEmail = 
        "To: " + to + "\r\n" +
        "Subject: " + subject + "\r\n" +
        "Content-Type: multipart/mixed; boundary=\"" + boundary + "\"\r\n\r\n" +
        "--" + boundary + "\r\n" +
        "Content-Type: text/plain; charset=\"UTF-8\"\r\n\r\n" +
        body + "\r\n\r\n" +
        "--" + boundary + "\r\n" +
        "Content-Type: " + mimeType + "; name=\"" + filePath + "\"\r\n" +
        "Content-Transfer-Encoding: base64\r\n" +
        "Content-Disposition: attachment; filename=\"" + filePath + "\"\r\n\r\n" +
        encodedFile + "\r\n" +
        "--" + boundary + "--";

    nlohmann::json emailJson;
    emailJson["raw"] = base64Encode(rawEmail);

    std::cout << "Finished encoded mail\n";

    std::string emailData = emailJson.dump();

    // Use a separate thread to send the file
    std::thread uploadThread([this, url, emailData, headers]() {
        this->sendFile(url, emailData, headers);
    });
    uploadThread.join();

    curl_slist_free_all(headers);
    curl_global_cleanup();
}

void GmailAPI::query(const std::string& query, const std::string& userName, const std::string& outputFile) {
    std::ofstream file(outputFile);
    try {
        std::string url = "https://www.googleapis.com/gmail/v1/users/me/messages?q=" + query + "&maxResults=25";
        if (!userName.empty()) {
            url += "+from:" + userName;
        }

        std::string readBuffer;
        CURL* curl = initializeCurl(url, tokenType, accessToken, readBuffer);
        if (!curl) {
            file << "Failed to initialize CURL\n";
            return;
        }

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            file << "CURL request failed: " << curl_easy_strerror(res) << "\n";
        } else {
            // Parse the JSON response
            auto jsonResponse = nlohmann::json::parse(readBuffer);
            if (jsonResponse.contains("messages")) {
                for (const auto& message : jsonResponse["messages"]) {
                    std::string messageId = message["id"];
                    std::string messageUrl = "https://www.googleapis.com/gmail/v1/users/me/messages/" + messageId;
                    
                    file << "Message ID: " << messageId << ":\n";
                    fetchMessageDetails(curl, messageUrl, readBuffer, file);
                }
            } else {
                file << "No messages found.\n";
            }
        }

        // Clean up
        curl_easy_cleanup(curl);
    } catch (const std::exception& e) {
        file << "An error occurred: " << e.what() << "\n";
    }

    file.close();
}

CURL* GmailAPI::initializeCurl(const std::string& url, const std::string& tokenType, const std::string& accessToken, std::string& readBuffer) {
    CURL* curl = curl_easy_init();

    if (!curl) return nullptr;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: " + tokenType + " " + accessToken).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    return curl;
}

void GmailAPI::fetchMessageDetails(CURL *curl, const std::string &messageUrl, std::string &readBuffer, std::ofstream &file)
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

            // Extract subject
            if (payload.contains("headers")) {
                for (const auto& header : payload["headers"]) {
                    if (header["name"] == "Subject") {
                        subject = header["value"];
                        break;
                    }
                }
            }

            // Extract body
            if (payload.contains("parts")) {
                for (const auto& part : payload["parts"]) {
                    if (part.contains("mimeType") && part["mimeType"] == "text/plain" && part.contains("body") && part["body"].contains("data")) {
                        body = base64Decode(part["body"]["data"]);
                        break;
                    }
                }
            }

            file << "Subject: " << subject << "\n";
            file << "Body: " << body << "\n\n";
        }
    } else {
        file << "Failed to fetch message details for URL: " << messageUrl << "\n";
    }
}

void GmailAPI::markAsRead() {

    std::vector<std::string> messageIds = extractMessageIds(messageListFileName);
    if (messageIds.empty()) {
        std::cout << "No unread messages\n";
        return;
    }

    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        for(const auto& messageId : messageIds) {
            std::string url = "https://www.googleapis.com/gmail/v1/users/me/messages/" + messageId + "/modify";
            std::string jsonData = "{\"removeLabelIds\": [\"UNREAD\"]}";

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
            else
                std::cout << "Message with ID " << messageId << " is successfully marked as read.\n";

            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

std::vector<std::string> GmailAPI::extractMessageIds(const std::string& filename) {
    std::vector<std::string> messageIds;
    std::ifstream file(filename);
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            if (line.find("Message ID:") == 0) {
                std::string messageId = line.substr(11);
                messageId.pop_back();
                messageId.erase(0, 1);
                messageIds.emplace_back(messageId);
            }
        }
        file.close();
    } else {
        std::cerr << "Unable to open file";
    }

    return messageIds;
}