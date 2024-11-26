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
    size_t chunkSize = input.size() / std::thread::hardware_concurrency();
    std::vector<std::future<std::string>> futures;
    std::string encoded;

    for (size_t i = 0; i < input.size(); i += chunkSize) {
        size_t end = i + chunkSize > input.size() ? input.size() : i + chunkSize;
        std::vector<unsigned char> chunk(input.begin() + i, input.begin() + end);
        futures.push_back(std::async(std::launch::async, [chunk]() { return encodeChunk(chunk); }));
    }

    for (auto& future : futures)
        encoded += future.get();

    return encoded;
}

std::string Base64::encodeChunk(const std::vector<unsigned char>& input) {
    std::string encoded;
    encoded.resize(4 * ((input.size() + 2) / 3));
    int encodedLength = EVP_EncodeBlock((unsigned char*)encoded.data(), (const unsigned char*)input.data(), static_cast<int>(input.size()));
    encoded.resize(encodedLength);
    return encoded;
}

std::string Base64::decode(const std::string &encoded_string) {
    int decodeLen = static_cast<int>(encoded_string.size() * 3 / 4);
    std::vector<unsigned char> buffer(decodeLen);

    int decoded_size = EVP_DecodeBlock(buffer.data(), reinterpret_cast<const unsigned char*>(encoded_string.data()), static_cast<int> (encoded_string.size()));
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

bool Base64::isBase64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}