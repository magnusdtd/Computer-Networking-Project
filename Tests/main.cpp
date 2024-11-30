#include <gtest/gtest.h>
#include <windows.h>

int main(int argc, char **argv) {
    SetConsoleOutputCP(CP_UTF8);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}