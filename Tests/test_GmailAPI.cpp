#include <gtest/gtest.h>
#include "./../GmailAPI/GmailAPI.hpp"

class GmailAPITest : public ::testing::Test {
protected:
    GmailAPI* gmailAPI;

    void SetUp() override {
        gmailAPI = new GmailAPI("./GmailAPI/oauth2.json", "./GmailAPI/token.json", "./GmailAPI/script-auto.ps1", "./GmailAPI/message-list.txt");
    }

    void TearDown() override {
        delete gmailAPI;
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

TEST_F(GmailAPITest, QueryEmails) {
    std::string query = "from:someone@example.com";
    std::string userName = "someone";

    // Call the query function
    ASSERT_NO_THROW(gmailAPI->query(query, userName));
}

TEST_F(GmailAPITest, MarkEmailsAsRead) {
    // Call the markAsRead function
    ASSERT_NO_THROW(gmailAPI->markAsRead());
}
