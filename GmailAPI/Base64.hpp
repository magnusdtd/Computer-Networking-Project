#ifndef BASE_64_HPP
#define BASE_64_HPP

#include <iostream>
#include <string>
#include <vector>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <thread>
#include <future>

class Base64 {
public:
    static std::string encode(const std::string &input);
    static std::string encode(const std::vector<unsigned char> &input);
    static std::string decode(const std::string &input);

private:
    static std::string encodeChunk(const std::vector<unsigned char>& input);
    static bool isBase64(unsigned char c);
};

#endif