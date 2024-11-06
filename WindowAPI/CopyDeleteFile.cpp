#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

// Function to copy a single file
bool copyFile(const wchar_t* source, const wchar_t* destination) {
    if (CopyFile(source, destination, FALSE)) {
        cout << "File copied successfully: " << source << endl;
        return true;
    }
    else {
        cout << "Failed to copy file: " << source << endl;
        return false;
    }
}

// Function to delete a file
bool deleteFile(const wchar_t* fileToDelete) {
    if (DeleteFile(fileToDelete)) {
        cout << "File deleted successfully: " << fileToDelete << endl;
        return true;
    }
    else {
        cout << "Failed to delete file: " << fileToDelete << endl;
        return false;
    }
}

// Function to create a new folder
bool createFolder(const wchar_t* folderPath) {
    if (CreateDirectory(folderPath, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        cout << "Folder created (or already exists): " << folderPath << endl;
        return true;
    }
    else {
        cout << "Failed to create folder: " << folderPath << endl;
        return false;
    }
}

// Function to copy an entire folder recursively
bool copyFolder(const wchar_t* sourceFolder, const wchar_t* destinationFolder) {
    WIN32_FIND_DATA findFileData;
    wchar_t sourceSearchPath[MAX_PATH];
    swprintf(sourceSearchPath, MAX_PATH, L"%s\\*.*", sourceFolder);

    HANDLE hFind = FindFirstFile(sourceSearchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        cout << "Failed to open source folder: " << sourceFolder << endl;
        return false;
    }

    // Create destination folder
    if (!createFolder(destinationFolder)) {
        FindClose(hFind);
        return false;
    }

    do {
        const wchar_t* fileName = findFileData.cFileName;

        // Skip "." and ".." entries
        if (wcscmp(fileName, L".") == 0 || wcscmp(fileName, L"..") == 0) {
            continue;
        }

        wchar_t sourcePath[MAX_PATH];
        wchar_t destPath[MAX_PATH];
        swprintf(sourcePath, MAX_PATH, L"%s\\%s", sourceFolder, fileName);
        swprintf(destPath, MAX_PATH, L"%s\\%s", destinationFolder, fileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Recursively copy subfolder
            if (!copyFolder(sourcePath, destPath)) {
                FindClose(hFind);
                return false;
            }
        }
        else {
            // Copy file
            if (!copyFile(sourcePath, destPath)) {
                FindClose(hFind);
                return false;
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return true;
}

int main() {
    const wchar_t* sourceFile = L"C:\\Users\\XUAN TRI\\source\\repos\\CopyDeleteFile\\testcase.txt";
    const wchar_t* destinationFile = L"C:\\Users\\XUAN TRI\\source\\repos\\CopyDeleteFile\\testcase2.txt";
    const wchar_t* sourceFolder = L"C:\\Users\\XUAN TRI\\source\\repos\\CopyDeleteFile\\NewFolder";
    const wchar_t* destinationFolder = L"C:\\Users\\XUAN TRI\\source\\repos\\CopyDeleteFile\\DestinationFolder";
    const wchar_t* fileToDelete = L"C:\\Users\\XUAN TRI\\source\\repos\\CopyDeleteFile\\testcase.txt";

    int choice;
    cout << "Choose an option:\n";
    cout << "1. Copy file\n";
    cout << "2. Copy file to a new folder\n";
    cout << "3. Copy entire folder\n";
    cin >> choice;

    if (choice == 1) {
        // Copy the file to the specified destination
        copyFile(sourceFile, destinationFile);
    }
    else if (choice == 2) {
        // Create a new folder and copy the file there
        if (createFolder(destinationFolder)) {
            wchar_t newDestination[MAX_PATH];
            swprintf(newDestination, MAX_PATH, L"%s\\testcase2.txt", destinationFolder);
            copyFile(sourceFile, newDestination);
        }
    }
    else if (choice == 3) {
        // Copy the entire folder
        copyFolder(sourceFolder, destinationFolder);
    }
    else {
        cout << "Invalid choice." << endl;
    }

    // Optionally delete the file (you can enable/disable as needed)
    deleteFile(fileToDelete);

    return 0;
}
