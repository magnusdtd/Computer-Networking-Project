#include <gtest/gtest.h>
#include "./../GmailAPI/GmailAPI.hpp"
#include "./../Client/ClientSocket.hpp"

class GmailAPITest : public ::testing::Test {
protected:
    GmailAPI* gmailAPI;
    ClientSocket* clientSocket;

    void SetUp() override {
        clientSocket = new ClientSocket();
        gmailAPI = new GmailAPI("./GmailAPI/oauth2.json", "./GmailAPI/token.json", "./GmailAPI/script-auto.ps1", "./GmailAPI/message-list.txt", *clientSocket);
    }

    void TearDown() override {
        delete gmailAPI;
        delete clientSocket;
    }
};

TEST_F(GmailAPITest, SendEmailWithSubjectAndBody) {
    std::string to = "datdt0212@gmail.com";
    std::string subject = "Test Subject";
    std::string body = "This is a test email body.";

    // Call the send function
    ASSERT_NO_THROW(gmailAPI->send(to, subject, body));
}

TEST_F(GmailAPITest, SendEmailWithSubjectBodyAndFile) {
    std::string to = "datdt0212@gmail.com";
    std::string subject = "Test Subject";
    std::string body = "This is a test email body.";
    std::string filePath = "./testfile.txt";

    // Create a test file
    std::ofstream testFile(filePath);
    testFile << "This is a test file.";
    testFile.close();

    // Call the send function
    ASSERT_NO_THROW(gmailAPI->send(to, subject, body, filePath));

    // Clean up the test file
    std::remove(filePath.c_str());
}

int main(int argc, char **argv) {   
    SetConsoleOutputCP(CP_UTF8);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}