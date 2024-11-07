#include <gtest/gtest.h>

// Sample test case
TEST(SampleTest, AssertionTrue) {
    ASSERT_TRUE(false);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}