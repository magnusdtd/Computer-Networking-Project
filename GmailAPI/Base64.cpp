#include "Base64.hpp"

std::string Base64::encode(const std::string &input) {
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

std::string Base64::encode(const std::vector<unsigned char>& input) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, reinterpret_cast<const char*>(input.data()), static_cast<int>(input.size()));
    BIO_flush(bio);

    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string encodedData(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return encodedData;
}

std::string Base64::decode(const std::string &encoded_string) {
    std::string normalized_encoded_string = normalizeBase64(encoded_string);
    int decodeLen = static_cast<int>(normalized_encoded_string.size() * 3 / 4);
    std::vector<unsigned char> buffer(decodeLen);

    int decoded_size = EVP_DecodeBlock(buffer.data(), reinterpret_cast<const unsigned char*>(normalized_encoded_string.data()), static_cast<int> (normalized_encoded_string.size()));
    if (decoded_size < 0) {
        std::cerr << "Failed to decode base64 input\n";
        return "";
    }

    while (decoded_size > 0 && buffer[decoded_size - 1] == '\0') {
        --decoded_size;
    }

    std::string decodedData(buffer.begin(), buffer.begin() + decoded_size);
    return decodedData;
}

std::string Base64::normalizeBase64(const std::string &input) {
    std::string normalized = input;
    // Replace URL-safe characters
    std::replace(normalized.begin(), normalized.end(), '-', '+');
    std::replace(normalized.begin(), normalized.end(), '_', '/');
    // Remove newline characters
    normalized.erase(std::remove(normalized.begin(), normalized.end(), '\n'), normalized.end());
    normalized.erase(std::remove(normalized.begin(), normalized.end(), '\r'), normalized.end());
    return normalized;
}