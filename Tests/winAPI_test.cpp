#include <gtest/gtest.h>
#include "./../WindowAPI/winAPI.hpp"
#include <fstream>
#include <filesystem>

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
    std::filesystem::remove_all(sourceFile);
    std::filesystem::remove_all(destFile);
}

TEST(WinAPITest, CopyFolder) {
    // Create a temporary source folder and file
    const wchar_t* sourceFolder = L"source_test_folder";
    const wchar_t* destFolder = L"dest_test_folder";

    {
        std::filesystem::create_directory(sourceFolder);
        std::filesystem::create_directory(destFolder);

        std::ofstream ofs(std::wstring(sourceFolder).append(L"/test_file.txt"));
        ofs << "Test file content.\n";
        ofs.close();
    }

    // Test copyFolder method
    std::string result;
    {
        WinAPI winAPI;
        result = winAPI.copyFolder(sourceFolder, destFolder);
    }
    EXPECT_EQ(result, "Folder copied successfully from source_test_folder to dest_test_folder");

    // Verify the copied folder and file
    std::filesystem::path copiedFolder = std::filesystem::path(destFolder) / L"source_test_folder";
    EXPECT_TRUE(std::filesystem::exists(copiedFolder));
    EXPECT_TRUE(std::filesystem::exists(copiedFolder / L"test_file.txt"));

    // Clean up
    std::filesystem::remove_all(sourceFolder);
    std::filesystem::remove_all(destFolder);
}


TEST(WinAPITest, ListProcesses) {
    WinAPI winAPI;

    // Test listProcesses method
    std::string result = winAPI.listProcesses();
    EXPECT_EQ(result, "Successfully list all processes.\n");

    // Verify the generated file
    std::filesystem::path outputDir = "output";
    EXPECT_TRUE(std::filesystem::exists(outputDir));

    // Find the generated file
    std::filesystem::path generatedFile;
    for (const auto& entry : std::filesystem::directory_iterator(outputDir)) {
        if (entry.path().extension() == ".txt" && entry.path().filename().string().find("process_list_") != std::string::npos) {
            generatedFile = entry.path();
            break;
        }
    }

    EXPECT_TRUE(!generatedFile.empty());
    EXPECT_TRUE(std::filesystem::exists(generatedFile));

    // Verify the file is not empty
    std::ifstream ifs(generatedFile);
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    EXPECT_FALSE(content.empty());

    // Clean up
    std::filesystem::remove(generatedFile);
}