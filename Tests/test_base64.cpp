#include <gtest/gtest.h>
#include "./../GmailAPI/Base64.hpp"

class Base64Test : public ::testing::Test {
protected:
    void SetUp() override {

    }

    void TearDown() override {

    }
};

TEST_F(Base64Test, EncodeString) {
    std::string input = "Hello, World!";
    std::string expectedOutput = "SGVsbG8sIFdvcmxkIQ==";
    std::string encoded = Base64::encode(input);
    ASSERT_EQ(encoded, expectedOutput);
}

TEST_F(Base64Test, DecodeString) {
    std::string input = "SGVsbG8sIFdvcmxkIQ==";
    std::string expectedOutput = "Hello, World!";
    std::string decoded = Base64::decode(input);
    ASSERT_EQ(decoded, expectedOutput);
}

TEST_F(Base64Test, EncodeVector) {
    std::vector<unsigned char> input = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
    std::string expectedOutput = "SGVsbG8sIFdvcmxkIQ==";
    std::string encoded = Base64::encode(input);
    ASSERT_EQ(encoded, expectedOutput);
}

TEST_F(Base64Test, DecodeInvalidString) {
    std::string input = "Invalid base64 string!";
    std::string decoded = Base64::decode(input);
    ASSERT_EQ(decoded, "");
}

TEST(Base64UnicodeTest, EncodeDecodeUnicode) {
    std::string unicodeStr = u8"👻";
    std::string encoded = Base64::encode(unicodeStr);
    std::string decoded = Base64::decode(encoded);
    ASSERT_EQ(decoded, unicodeStr);
}

TEST(Base64UnicodeTest, EncodeDecodeUnicode_2) {
    std::string unicodeStr = "This is the command I need to execute it right now 👻 STOP ok dfdfa";
    std::string encodedExpected = "VGhpcyBpcyB0aGUgY29tbWFuZCBJIG5lZWQgdG8gZXhlY3V0ZSBpdCByaWdodCBub3cg8J+RuyBTVE9QIG9rIGRmZGZh";
    std::string gmailEncoded = "VGhpcyBpcyB0aGUgY29tbWFuZCBJIG5lZWQgdG8gZXhlY3V0ZSBpdCByaWdodCBub3cg8J-RuyBTVE9QIG9rIGRmZGZhDQo=";
    std::string encoded = Base64::encode(unicodeStr);
    std::string decoded = Base64::decode(encoded);
    ASSERT_EQ(encoded, encodedExpected);
    ASSERT_EQ(decoded, unicodeStr);
}