#ifndef WIN_API_HPP
#define WIN_API_HPP

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
#include <shellapi.h>
#include <tlhelp32.h>
#include "Keylogger.hpp"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "gdiplus.lib")

class WinAPI {

    std::string wcharToString(const wchar_t* wstr);

    void initializeGDIPlus();

    static HHOOK hKeyboardHook;

    static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

    static HHOOK hHook;

public:

    WinAPI() {
        initializeGDIPlus();
    }

    // Generate a unique file name using the current time
    std::string generateName(const std::string prefixName, const std::string extensionName);

    BOOL systemShutdown();

    BOOL systemRestart(LPWSTR lpMsg);

    // Capture and save screenshot
    std::string saveScreenshot(const std::string&  filePath);

    // Copy a single file  at source to the given destination
    std::string copyFile(const wchar_t* source, const wchar_t* destination);

    // Delete a file
    std::string deleteFile(const wchar_t* fileToDelete);

    // Create a new folder
    std::string createFolder(const wchar_t* folderPath);

    // Copy an entire folder recursively
    std::string copyFolder(const wchar_t* sourceFolder, const wchar_t* destinationFolder);

    std::string StartApplication(const std::wstring& applicationPath);

    std::string TerminateProcessByID(DWORD processID);

    //app = process has a visiable window
    std::string listRunningApp(std::string &filePath);

    std::string listInstalledApp(std::string &filePath);

    std::string listFilesInDirectory(const std::wstring &directoryPath, std::string &filePath);

    void disableKeyboard();

    void removeKeyboardHook();
};

#endif