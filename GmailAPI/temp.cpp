void GmailAPI::send(const std::string &to, const std::string &subject, const std::string &body, const std::string &filePath) {
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