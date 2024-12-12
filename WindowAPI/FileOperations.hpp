#ifndef FILE_OPERATIONS_HPP
#define FILE_OPERATIONS_HPP

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <iostream>
#include <stack>
#include "./../WindowAPI/MyUtility.hpp"

class FileOperations {
public:
    std::string copyFile(const wchar_t* source, const wchar_t* destination);
    std::string deleteFile(const wchar_t* fileToDelete);
    std::string createFolder(const wchar_t* folderPath);
    std::string copyFolder(const wchar_t* sourceFolder, const wchar_t* destinationFolder);
    std::string listFilesInDirectory(const std::wstring& directoryPath, std::string& filePath);
};

#endif