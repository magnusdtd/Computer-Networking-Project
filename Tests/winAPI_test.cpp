#include <gtest/gtest.h>
#include "./../WindowAPI/winAPI.hpp"

TEST(WinAPITest, DeleteFile) {
    WinAPI winAPI;

    // Create a temporary file
    const wchar_t* tempFile = L"temp_test_file.txt";
    std::ofstream ofs(tempFile);
    ofs << "Temporary file content";
    ofs.close();

    // Test deleteFile method
    std::string result = winAPI.deleteFile(tempFile);
    EXPECT_EQ(result, "File deleted successfully: temp_test_file.txt");
}

TEST(WinAPITest, CreateFolder) {
    WinAPI winAPI;

    // Create a temporary folder
    const wchar_t* tempFolder = L"temp_test_folder";

    // Test createFolder method
    std::string result = winAPI.createFolder(tempFolder);
    EXPECT_EQ(result, "Folder created (or already exists): temp_test_folder");

    // Clean up
    RemoveDirectoryW(tempFolder);
}

TEST(WinAPITest, CopyFile) {
    WinAPI winAPI;

    // Create a temporary source file
    const wchar_t* sourceFile = L"source_test_file.txt";
    std::ofstream ofs(sourceFile);
    ofs << "Source file content";
    ofs.close();

    // Define destination file
    const wchar_t* destFile = L"dest_test_file.txt";

    // Test copyFile method
    std::string result = winAPI.copyFile(sourceFile, destFile);
    EXPECT_EQ(result, "File copied successfully: source_test_file.txt");

    // Clean up
    winAPI.deleteFile(sourceFile);
    winAPI.deleteFile(destFile);
}

TEST(WinAPITest, CopyFolder) {
    WinAPI winAPI;

    // Create a temporary source folder and file
    const wchar_t* sourceFolder = L"source_test_folder";
    const wchar_t* destFolder = L"dest_test_folder";
    winAPI.createFolder(sourceFolder);
    std::ofstream ofs(std::wstring(sourceFolder).append(L"\\test_file.txt"));
    ofs << "Test file content";
    ofs.close();

    // Test copyFolder method
    bool result = winAPI.copyFolder(sourceFolder, destFolder);
    EXPECT_TRUE(result);

    // Clean up
    std::filesystem::remove_all(sourceFolder);
    std::filesystem::remove_all(destFolder);
}