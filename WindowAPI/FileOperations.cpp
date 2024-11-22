#include "FileOperations.hpp"

std::string FileOperations::copyFile(const wchar_t* source, const wchar_t* destination) {
    std::string result;
    if (CopyFileW(source, destination, FALSE)) {
        std::string str = MyUtility::wcharToString(source);
        result = "File copied successfully: " + str;
    } else {
        result = "Failed to copy file: " + MyUtility::wcharToString(source);
        std::cout << result << '\n';
    }
    return result;
}

std::string FileOperations::deleteFile(const wchar_t* fileToDelete) {
    std::string result;
    if (DeleteFileW(fileToDelete)) {
        std::string str = MyUtility::wcharToString(fileToDelete);
        result = "File deleted successfully: " + str;
    } else {
        result = "Failed to delete file: " + MyUtility::wcharToString(fileToDelete);
        std::cout << result << '\n';
    }
    return result;
}

std::string FileOperations::createFolder(const wchar_t* folderPath) {
    std::string result;
    if (CreateDirectoryW(folderPath, nullptr) || GetLastError() == ERROR_ALREADY_EXISTS) {
        std::string str = MyUtility::wcharToString(folderPath);
        result = "Folder created (or already exists): " + str;
    } else {
        result = "Failed to create folder: " + MyUtility::wcharToString(folderPath);
        std::cout << result << '\n';
    }
    return result;
}

std::string FileOperations::copyFolder(const wchar_t* sourceFolder, const wchar_t* destinationFolder) {
    std::filesystem::path sourcePath(sourceFolder);
    std::filesystem::path destPath(destinationFolder);

    // Convert relative paths to absolute paths
    sourcePath = std::filesystem::absolute(sourcePath);
    destPath = std::filesystem::absolute(destPath) / sourcePath.filename();

    WIN32_FIND_DATAW findFileData;
    wchar_t sourceSearchPath[MAX_PATH];
    swprintf(sourceSearchPath, MAX_PATH, L"%s\\*.*", sourcePath.c_str());

    HANDLE hFind = FindFirstFileW(sourceSearchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        std::wcout << L"Failed to open source folder: " << sourcePath << '\n';
        return "Failed to open source folder: " + MyUtility::wcharToString(sourcePath.c_str());
    }

    // Create destination folder
    if (!CreateDirectoryW(destPath.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        FindClose(hFind);
        return "Failed to create destination folder: " + MyUtility::wcharToString(destPath.c_str());
    }

    do {
        const wchar_t* fileName = findFileData.cFileName;

        // Skip "." and ".." entries
        if (wcscmp(fileName, L".") == 0 || wcscmp(fileName, L"..") == 0) {
            continue;
        }

        std::filesystem::path sourceFilePath = sourcePath / fileName;
        std::filesystem::path destFilePath = destPath / fileName;

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Recursively copy subfolder
            std::string result = copyFolder(sourceFilePath.c_str(), destFilePath.c_str());
            if (result.find("Failed") != std::string::npos) {
                FindClose(hFind);
                return result;
            }
        } else {
            // Copy file
            if (!CopyFileW(sourceFilePath.c_str(), destFilePath.c_str(), FALSE)) {
                FindClose(hFind);
                return "Failed to copy file: " + MyUtility::wcharToString(sourceFilePath.c_str());
            }
        }
    } while (FindNextFileW(hFind, &findFileData) != 0);

    FindClose(hFind);
    return "Folder copied successfully from " + MyUtility::wcharToString(sourceFolder) + " to " + MyUtility::wcharToString(destinationFolder);
}

std::string FileOperations::listFilesInDirectory(const std::wstring& directoryPath, std::string& filePath) {
    std::ofstream out(filePath);

    if (!out.is_open()) {
        std::cerr << "Failed to open output file: " << filePath << '\n';
        return "Failed to open output file!";
    }

    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW((directoryPath + L"\\*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open directory: " << MyUtility::wcharToString(directoryPath.c_str()) << '\n';
        return "Failed to open directory!";
    }

    do {
        const std::wstring fileName = findFileData.cFileName;
        if (fileName != L"." && fileName != L"..") {
            out << "File: " << MyUtility::wcharToString(fileName.c_str()) << '\n';
        }
    } while (FindNextFileW(hFind, &findFileData) != 0);

    FindClose(hFind);
    out.close();

    return "Successfully listed all files in directory at " + filePath;
}