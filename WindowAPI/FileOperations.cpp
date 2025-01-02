#include "FileOperations.hpp"

std::string FileOperations::copyFile(const wchar_t* source, const wchar_t* destination) {
    std::string result;
    try {
        std::filesystem::path sourcePath(source);
        std::filesystem::path destinationPath(destination);

        // Copy the file
        std::filesystem::copy_file(sourcePath, destinationPath, std::filesystem::copy_options::overwrite_existing);

        result = "File copied successfully: " + MyUtility::wcharToString(source);
    } catch (const std::filesystem::filesystem_error& e) {
        result = "Failed to copy file: " + MyUtility::wcharToString(source) + " Error: " + e.what();
        std::cout << result << '\n';
    }
    return result;
}

std::string FileOperations::deleteFile(const wchar_t* fileToDelete) {
    std::string result;
    try {
        std::filesystem::path filePath(fileToDelete);

        // Delete the file
        std::filesystem::remove(filePath);

        result = "File deleted successfully: " + MyUtility::wcharToString(fileToDelete);
    } catch (const std::filesystem::filesystem_error& e) {
        result = "Failed to delete file: " + MyUtility::wcharToString(fileToDelete) + " Error: " + e.what();
        std::cout << result << '\n';
    }
    return result;
}

std::string FileOperations::createFolder(const wchar_t* folderPath) {
    std::string result;
    try {
        std::filesystem::path path(folderPath);

        // Create the directory
        if (std::filesystem::create_directories(path)) {
            result = "Folder created: " + MyUtility::wcharToString(folderPath);
        } else {
            result = "Folder already exists: " + MyUtility::wcharToString(folderPath);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        result = "Failed to create folder: " + MyUtility::wcharToString(folderPath) + " Error: " + e.what();
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

    std::wcout << L"Source Path: " << sourcePath << '\n';
    std::wcout << L"Destination Path: " << destPath << '\n';

    try {
        // Check if source path exists and is a directory
        if (!std::filesystem::exists(sourcePath) || !std::filesystem::is_directory(sourcePath)) {
            std::wcout << L"Failed to open source folder: " << sourcePath << L" Error: Path not found or not a directory\n";
            return "Failed to open source folder: " + MyUtility::wcharToString(sourcePath.c_str()) + " Error: Path not found or not a directory";
        }

        // Create destination directory
        std::filesystem::create_directories(destPath);

        // Iterate through the source directory
        for (const auto& entry : std::filesystem::recursive_directory_iterator(sourcePath)) {
            const auto& path = entry.path();
            auto relativePathStr = path.lexically_relative(sourcePath).wstring();
            std::filesystem::path dest = destPath / relativePathStr;

            if (entry.is_directory()) {
                // Create directory
                std::filesystem::create_directories(dest);
            } else if (entry.is_regular_file()) {
                // Copy file
                std::filesystem::copy_file(path, dest, std::filesystem::copy_options::overwrite_existing);
            } else {
                std::wcout << L"Skipping non-regular file: " << path << '\n';
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::wcout << L"Filesystem error: " << e.what() << '\n';
        return "Filesystem error: " + std::string(e.what());
    } catch (const std::exception& e) {
        std::wcout << L"General error: " << e.what() << '\n';
        return "General error: " + std::string(e.what());
    }

    return "Folder copied successfully from " + MyUtility::wcharToString(sourceFolder) + " to " + MyUtility::wcharToString(destinationFolder);
}

std::string FileOperations::listFilesInDirectory(const std::wstring& directoryPath, std::string& filePath) {
    std::ofstream out(filePath);

    if (!out.is_open()) {
        std::cerr << "Failed to open output file: " << filePath << '\n';
        return "Failed to open output file!";
    }

    // Write CSV header
    out << "Type,Name\n";

    try {
        std::filesystem::path dirPath(directoryPath);

        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
            std::cerr << "Failed to open directory: " << MyUtility::wcharToString(directoryPath.c_str()) << '\n';
            return "Failed to open directory!";
        }

        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            const auto& path = entry.path();
            if (entry.is_directory()) {
                out << "\"DIR\",\"" << MyUtility::wcharToString(path.filename().wstring().c_str()) << "\"\n";
            } else if (entry.is_regular_file()) {
                out << "\"FILE\",\"" << MyUtility::wcharToString(path.filename().wstring().c_str()) << "\"\n";
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
        return "Filesystem error: " + std::string(e.what());
    } catch (const std::exception& e) {
        std::cerr << "General error: " << e.what() << '\n';
        return "General error: " + std::string(e.what());
    }

    out.close();
    return "Successfully listed files in directory at " + filePath;
}