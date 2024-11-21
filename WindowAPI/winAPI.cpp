#include "winAPI.hpp"

HHOOK WinAPI::hHook = nullptr;
HHOOK WinAPI::hKeyboardHook = nullptr;

std::string WinAPI::generateName(const std::string prefixName, const std::string extensionName) {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    tm timeInfo;
    localtime_s(&timeInfo, &in_time_t);
    ss << std::put_time(&timeInfo, "%Y%m%d%H%M%S");
    std::string fileName = prefixName + '_' + ss.str() + '.' + extensionName;
    return fileName;
}

std::string WinAPI::WinAPI::wcharToString(const wchar_t* wstr) {
    if (!wstr) return "";

    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (bufferSize == 0)
        return "";

    std::vector<char> buffer(bufferSize);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buffer.data(), bufferSize, nullptr, nullptr);

    return std::string(buffer.data());
}

BOOL WinAPI::systemShutdown()
{
    HANDLE hToken; 
    TOKEN_PRIVILEGES tkp; 

    // Get a token for this process. 
    if (!OpenProcessToken(GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
        return FALSE; 

    // Get the LUID for the shutdown privilege. 
    LookupPrivilegeValue(nullptr, SE_SHUTDOWN_NAME, 
        &tkp.Privileges[0].Luid); 

    tkp.PrivilegeCount = 1;  // one privilege to set    
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

    // Get the shutdown privilege for this process.  
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
        (PTOKEN_PRIVILEGES)nullptr, 0); 

    if (GetLastError() != ERROR_SUCCESS) 
        return FALSE; 

    // Shut down the system and force all applications to close. 
    if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 
                SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
                SHTDN_REASON_MINOR_UPGRADE |
                SHTDN_REASON_FLAG_PLANNED)) 
        return FALSE; 

    return TRUE;
}

BOOL WinAPI::systemRestart(LPWSTR lpMsg)
{
    HANDLE hToken;              // handle to process token 
    TOKEN_PRIVILEGES tkp;       // pointer to token structure 

    BOOL fResult;               // system shutdown flag 

    // Get the current process token handle so we can get shutdown 
    // privilege. 

    if (!OpenProcessToken(GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
        return FALSE; 

    // Get the LUID for shutdown privilege. 

    LookupPrivilegeValue(nullptr, SE_SHUTDOWN_NAME, 
        &tkp.Privileges[0].Luid); 

    tkp.PrivilegeCount = 1;  // one privilege to set    
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

    // Get shutdown privilege for this process. 

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
        (PTOKEN_PRIVILEGES) nullptr, 0); 

    // Cannot test the return value of AdjustTokenPrivileges. 

    if (GetLastError() != ERROR_SUCCESS) 
        return FALSE; 

    // Display the shutdown dialog box and start the countdown. 

    fResult = InitiateSystemShutdownW( 
        nullptr,    // shut down local computer 
        lpMsg,   // message for user
        0,      // time-out period, in seconds 
        TRUE,   // ask user to close apps 
        TRUE);   // reboot after shutdown 

    if (!fResult) 
        return FALSE; 

    // Disable shutdown privilege. 
    tkp.Privileges[0].Attributes = 0; 
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
        (PTOKEN_PRIVILEGES) nullptr, 0); 

    return TRUE; 
}

void WinAPI::initializeGDIPlus()
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::Status status = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    if (status == Gdiplus::Ok)
        std::cout << "GDIPlus successfully initialized.\n";
    else
        std::cout << "Failed to initialize GDI+. Status: " << status << "\n";
}

std::string WinAPI::saveScreenshot(const std::string& filePath) 
{
    HWND hwnd = GetDesktopWindow();
    HDC hdcWindow = GetDC(hwnd);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);

    RECT rc;
    GetWindowRect(hwnd, &rc);

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, rc.right - rc.left, rc.bottom - rc.top);
    SelectObject(hdcMemDC, hBitmap);

    BitBlt(hdcMemDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcWindow, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, hdcWindow);

    if (!hBitmap) {
        std::cout << "Failed to capture screen.\n";
        return "Failed to capture screen.";
    }

    std::vector<BYTE> buf;
    IStream* stream = nullptr;
    HRESULT hr = CreateStreamOnHGlobal(0, TRUE, &stream);

    if (FAILED(hr)) {
        std::cout << "Failed to create stream.\n";
        DeleteObject(hBitmap);
        DeleteDC(hdcMemDC);
        return "Failed to create stream.";
    }

    CImage image;
    ULARGE_INTEGER liSize;
    image.Attach(hBitmap);
    hr = image.Save(stream, Gdiplus::ImageFormatPNG);

    if (FAILED(hr)) {
        std::cout << "Failed to save image to stream.\n";
        stream->Release();
        DeleteObject(hBitmap);
        DeleteDC(hdcMemDC);
        return "Failed to save image to stream.";
    }

    IStream_Size(stream, &liSize);
    DWORD len = liSize.LowPart;
    IStream_Reset(stream);
    buf.resize(len);
    IStream_Read(stream, &buf[0], len);
    stream->Release();

    std::fstream fileBuffer;
    fileBuffer.open(filePath, std::fstream::binary | std::fstream::out);
    if (!fileBuffer.is_open()) {
        std::cout << "Failed to open file for writing.\n";
        DeleteObject(hBitmap);
        DeleteDC(hdcMemDC);
        return "Failed to open file for writing.\n";
    }

    fileBuffer.write(reinterpret_cast<const char*>(&buf[0]), buf.size() * sizeof(BYTE));
    fileBuffer.close();

    DeleteObject(hBitmap);
    DeleteDC(hdcMemDC);

    return "Capture screen successful, new file at " + filePath ;
}

std::string WinAPI::copyFile(const wchar_t* source, const wchar_t* destination) {
    std::string result;
    if (CopyFileW(source, destination, FALSE)) {
        std::string str = WinAPI::wcharToString(source);
        result = "File copied successfully: " + str;
    } else {
        result = "Failed to copy file: " + WinAPI::wcharToString(source);
        std::cout << result << '\n';
    }
    return result;
}

std::string WinAPI::deleteFile(const wchar_t* fileToDelete) {
    std::string result;
    if (DeleteFileW(fileToDelete)) {
        std::string str = WinAPI::wcharToString(fileToDelete);
        result = "File deleted successfully: " + str;
    } else {
        result = "Failed to delete file: " + WinAPI::wcharToString(fileToDelete);
        std::cout << result << '\n';
    }
    return result;
}

std::string WinAPI::createFolder(const wchar_t* folderPath) {
    std::string result;
    if (CreateDirectoryW(folderPath, nullptr) || GetLastError() == ERROR_ALREADY_EXISTS) {
        std::string str = WinAPI::wcharToString(folderPath);
        result = "Folder created (or already exists): " + str;
    } else {
        result = "Failed to create folder: " + WinAPI::wcharToString(folderPath);
        std::cout << result << '\n';
    }
    return result;
}

std::string WinAPI::copyFolder(const wchar_t* sourceFolder, const wchar_t* destinationFolder) 
{
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
        return "Failed to open source folder: " + wcharToString(sourcePath.c_str());
    }

    // Create destination folder
    if (!CreateDirectoryW(destPath.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        FindClose(hFind);
        return "Failed to create destination folder: " + wcharToString(destPath.c_str());
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
                return "Failed to copy file: " + wcharToString(sourceFilePath.c_str());
            }
        }
    } while (FindNextFileW(hFind, &findFileData) != 0);

    FindClose(hFind);
    return "Folder copied successfully from " + wcharToString(sourceFolder) + " to " + wcharToString(destinationFolder);
}

std::string WinAPI::StartApplication(const std::wstring& applicationPath) {
    HINSTANCE result = ShellExecuteW(NULL, L"open", applicationPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if ((intptr_t)result <= 32) {
        std::wcerr << L"Failed to start application. Error code: " << (intptr_t)result << std::endl;
        return "Failed";
    }
    return "Succeeded start application with path: " + wcharToString(applicationPath.c_str());
}

std::string WinAPI::TerminateProcessByID(DWORD processID) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
    std::string result = "Succeeded to close process with ID: " + std::to_string(processID) + '\n';
    if (hProcess == nullptr) {
        result = "Failed to open process with ID " + std::to_string(processID) + "\n";
        std::cerr << result;
        return result;
    }

    if (!TerminateProcess(hProcess, 0)) {
        result = "Failed to terminate process " + std::to_string(processID) + "\n";
        std::cerr << result;
        CloseHandle(hProcess);
        return "Failed";
    }

    CloseHandle(hProcess);
    return result;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);  // Get the PID of the window

    // If the process ID matches the one passed in lParam, check if the window is visible
    if (processId == lParam) {
        if (IsWindowVisible(hwnd)) {
            // This process has a visible window
            return FALSE;  // Stop enumeration since we found a window
        }
    }
    return TRUE;  // Continue enumeration
}

std::string WinAPI::listRunningApp(std::string &filePath)
{
    std::ofstream out(filePath);
    if (!out.is_open()) {
        std::cerr << "Failed to open output file: " << filePath << '\n';
        return "Failed to open output file!";
    }

    std::string result;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        result = "Failed to create snapshot!\n";
        std::cerr << result;
        return result;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &pe)) {
        do {
            DWORD processId = pe.th32ProcessID;
            if (processId != 0) { 
                BOOL hasWindow = EnumWindows(EnumWindowsProc, (LPARAM)processId);
                if (!hasWindow) {
                    out << "Process ID: " << processId << " - " << pe.szExeFile << '\n';
                }
            }
        } while (Process32Next(snapshot, &pe));
    }
    else {
        result = "Failed to retrieve process information!\n";
        std::cerr << result;
        return result;
    }

    CloseHandle(snapshot);
    out.close();
    result = "Successfully listed all running applications at " + filePath;
    return result;
}

std::string WinAPI::listInstalledApp(std::string &filePath)
{
    std::ofstream out(filePath);

    if (!out.is_open()) {
        std::cerr << "Failed to open output file: " << filePath << '\n';
        return "Failed to open output file!";
    }

    std::string result;
    HKEY hKey;
    const char* subKeys[] = {
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
        "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    };

    for (const char* subKey : subKeys) {
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD index = 0;
            char keyName[256];
            DWORD keyNameSize = sizeof(keyName);

            while (RegEnumKeyExA(hKey, index, keyName, &keyNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
                HKEY hSubKey;
                if (RegOpenKeyExA(hKey, keyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    char displayName[256];
                    char installLocation[256];
                    char uninstallString[256];
                    DWORD displayNameSize = sizeof(displayName);
                    DWORD installLocationSize = sizeof(installLocation);
                    DWORD uninstallStringSize = sizeof(uninstallString);

                    if (RegQueryValueExA(hSubKey, "DisplayName", nullptr, nullptr, (LPBYTE)displayName, &displayNameSize) == ERROR_SUCCESS) {
                        if (RegQueryValueExA(hSubKey, "InstallLocation", nullptr, nullptr, (LPBYTE)installLocation, &installLocationSize) != ERROR_SUCCESS) {
                            strcpy_s(installLocation, "Unknown");
                        }
                        if (RegQueryValueExA(hSubKey, "UninstallString", nullptr, nullptr, (LPBYTE)uninstallString, &uninstallStringSize) != ERROR_SUCCESS) {
                            strcpy_s(uninstallString, "Unknown");
                        }

                        std::string exePath = installLocation;
                        if (exePath == "Unknown" || exePath.empty()) {
                            exePath = uninstallString;
                        }

                        out << "Application: " << displayName << " - Path: " << exePath << '\n';
                    }
                    RegCloseKey(hSubKey);
                }
                keyNameSize = sizeof(keyName);
                index++;
            }
            RegCloseKey(hKey);
        } else {
            std::cerr << "Failed to open registry key: " << subKey << '\n';
        }
    }

    out.close();
    result = "Successfully listed all installed applications at " + filePath;
    return result;
}

std::string WinAPI::listFilesInDirectory(const std::wstring& directoryPath, std::string& filePath) {
    std::ofstream out(filePath);

    if (!out.is_open()) {
        std::cerr << "Failed to open output file: " << filePath << '\n';
        return "Failed to open output file!";
    }

    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW((directoryPath + L"\\*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open directory: " << wcharToString(directoryPath.c_str()) << '\n';
        return "Failed to open directory!";
    }

    do {
        const std::wstring fileName = findFileData.cFileName;
        if (fileName != L"." && fileName != L"..") {
            out << "File: " << wcharToString(fileName.c_str()) << '\n';
        }
    } while (FindNextFileW(hFind, &findFileData) != 0);

    FindClose(hFind);
    out.close();

    return "Successfully listed all files in directory at " + filePath;
}

LRESULT CALLBACK WinAPI::KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0) {
        return CallNextHookEx(hHook, nCode, wParam, lParam);
    }

    if (wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        return 1;
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void WinAPI::disableKeyboard() {
    hHook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);
    if (hHook == NULL) {
        std::cerr << "Failed to set keyboard hook. Error: " << GetLastError() << '\n';
        return;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hHook);
}

void WinAPI::removeKeyboardHook() {
    if (hKeyboardHook) {
        UnhookWindowsHookEx(hKeyboardHook);
        hKeyboardHook = NULL;
    }
}