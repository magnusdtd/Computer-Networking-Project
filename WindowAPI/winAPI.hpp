#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <sstream>
#include <time.h>
#include <iomanip>
#include <windows.h>
#include <atlimage.h>
#include <gdiplus.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "gdiplus.lib")

namespace WinAPI {

    std::string wcharToString(const wchar_t* wstr);

    BOOL systemShutdown();

    BOOL systemRestart(LPWSTR lpMsg);

    void initializeGDIPlus();

    // Capture and save screenshot
    std::string saveScreenshot();

    // Copy a single file  at source to the given destination
    std::string copyFile(const wchar_t* source, const wchar_t* destination);

    // Delete a file
    std::string deleteFile(const wchar_t* fileToDelete);

    // Create a new folder
    std::string createFolder(const wchar_t* folderPath);

    // Copy an entire folder recursively
    bool copyFolder(const wchar_t* sourceFolder, const wchar_t* destinationFolder);
}