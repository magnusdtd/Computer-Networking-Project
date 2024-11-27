#include "GmailAPI.hpp"

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

GmailAPI::GmailAPI(const std::string& oauthFilePath, const std::string& tokenFilePath, const std::string& scriptFilePath, const std::string& messageListFilePath)
    : OAuthManager(oauthFilePath, tokenFilePath, scriptFilePath), 
      base64(new Base64()), 
      messageListFilePath(messageListFilePath) {}

GmailAPI::~GmailAPI()
{
    delete base64;
    base64 = nullptr;
}

void GmailAPI::send(const std::string &to, const std::string &subject, const std::string &body)
{
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
    std::string encodedEmail = base64->encode(rawEmail);
    emailJson["raw"] = encodedEmail;
    std::string emailData = emailJson.dump();

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, emailData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
    } else {
        std::cout << "Email sent successfully!\n";
        std::cout << "Response: " << readBuffer << '\n';
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();
}

void GmailAPI::send(const std::string &to, const std::string &subject, const std::string &body, const std::string &filePath)
{
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
        encodedFile = base64->encode(fileContent);
    }
    else if (fileExtension == "txt") {
        mimeType = "text/plain";
        std::string fileContent = readFile(filePath);
        encodedFile = base64->encode(fileContent);
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
        encodedFile = base64->encode(fileContent);
    }
    else {
        std::cerr << "Unsupported file type\n";
        return;
    }

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
    emailJson["raw"] = base64->encode(rawEmail);

    std::string emailData = emailJson.dump();

    // Use a separate thread to send the file
    std::thread uploadThread([this, url, emailData, headers, &readBuffer]() {
        CURL *curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, emailData.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
            } else {
                std::cout << "Email sent successfully!\n";
                std::cout << "Response: " << readBuffer << '\n';
            }

            curl_easy_cleanup(curl);
        }
    });
    uploadThread.join();

    curl_slist_free_all(headers);
    curl_global_cleanup();
}

void GmailAPI::query(const std::string& query, const std::string& userName) {
    std::ofstream file(messageListFilePath, std::ios::out | std::ios::binary);
    try {
        // Check if the token is expired and refresh if necessary
        if (isTokenExpired())
            getRefreshToken();

        CURL* curl = curl_easy_init();
        if (!curl) {
            file << "Failed to initialize CURL\n";
            return;
        }

        char* encodedQuery = curl_easy_escape(curl, query.c_str(), static_cast<int>(query.length()));
        std::string url = "https://www.googleapis.com/gmail/v1/users/me/messages?q=" + std::string(encodedQuery);
        curl_free(encodedQuery);

        if (!userName.empty()) {
            char* encodedUserName = curl_easy_escape(curl, userName.c_str(), static_cast<int>(userName.length()));
            url += "+from:" + std::string(encodedUserName);
            curl_free(encodedUserName);
        }
        url += "&maxResults=25";

        std::string readBuffer;
        curl = initializeCurl(url, tokenType, accessToken, readBuffer);
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

            // std::ofstream jsonFile("./GmailAPI/response.json", std::ios::out);
            // jsonFile << readBuffer << "\n";
            // jsonFile.close();

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

    // std::ofstream jsonFile("./GmailAPI/response.json", std::ios::out);
    // jsonFile << readBuffer << "\n";
    // jsonFile.close();

    if (res == CURLE_OK) {
        auto messageResponse = nlohmann::json::parse(readBuffer);
        if (messageResponse.contains("payload")) {
            const auto& payload = messageResponse["payload"];
            std::string subject;
            std::string body;
            std::string sender;
            std::string senderEmail;

            // Extract subject, sender, and sender email
            if (payload.contains("headers")) {
                for (const auto& header : payload["headers"]) {
                    if (header["name"] == "Subject") {
                        subject = header["value"];
                    } else if (header["name"] == "From") {
                        sender = header["value"];
                        size_t start = sender.find('<');
                        size_t end = sender.find('>');
                        if (start != std::string::npos && end != std::string::npos) {
                            senderEmail = sender.substr(start + 1, end - start - 1);
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

            file << "From: " << sender << "\n";
            file << "Sender email: " << senderEmail << "\n";
            file << "Subject: " << subject << "\n";
            file << "Body: " << body << "\n\n";
            
        }
    } else {
        file << "Failed to fetch message details for URL: " << messageUrl << "\n";
    }
}

void GmailAPI::markAsRead() {

    std::vector<std::string> messageIds = extractMessageIds(messageListFilePath);
    if (messageIds.empty()) {
        std::cout << "\t No unread messages\n";
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
