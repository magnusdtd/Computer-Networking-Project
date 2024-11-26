#include <gtest/gtest.h>
#include "./../GmailAPI/GmailAPI.hpp"

TEST(GmailAPITest, Base64Decode) {
    GmailAPI gmailAPI("./GmailAPI/oauth2.json", "./GmailAPI/token.json", "./GmailAPI/script-auto.ps1", "./GmailAPI/message-list.txt");

    std::string input = "YWZoYWtsZmhhZiBhbGZqYWRmbGogcnlyYXNnDQphZWVmZQ0KZGZnc2Rnag0Kd3JncndnIPCfkbsgU1RPUCBvaw0KZnNrZWhqZg0KDQpieWUsIPCfpKfij6zirIfvuI_wn6m18J-YjQ0K";
    std::string expectedOutput = "afhaklfhaf alfjadflj ryrasg\naeefe\ndfgsdgj\nwrgrwg 👻 STOP ok\nfskehjf\n\nbye, 🤧⏬⬇️🩵😍\n";

    std::string output = gmailAPI.base64Decode(input);

    ASSERT_EQ(output, expectedOutput);
}

int main(int argc, char **argv) {   
    SetConsoleOutputCP(CP_UTF8);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}