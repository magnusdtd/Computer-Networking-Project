#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <sstream>
#include <time.h>
#include <iomanip>
#include <filesystem>
#include <windows.h>
#include <atlimage.h>
#include <gdiplus.h>
#include <psapi.h>
#include <tchar.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "gdiplus.lib")

class WinAPI {

    std::string wcharToString(const wchar_t* wstr);

    void initializeGDIPlus();

    void printProcessNameAndID(DWORD processID, std::wofstream& file);

public:

    WinAPI() {
        initializeGDIPlus();
    }

    BOOL systemShutdown();

    BOOL systemRestart(LPWSTR lpMsg);

    // Capture and save screenshot
    std::string saveScreenshot();

    // Copy a single file  at source to the given destination
    std::string copyFile(const wchar_t* source, const wchar_t* destination);

    // Delete a file
    std::string deleteFile(const wchar_t* fileToDelete);

    // Create a new folder
    std::string createFolder(const wchar_t* folderPath);

    // Copy an entire folder recursively
    std::string copyFolder(const wchar_t* sourceFolder, const wchar_t* destinationFolder);

    std::string listProcesses();

};