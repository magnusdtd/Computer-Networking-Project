#include "winAPI.hpp"

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
            15,      // time-out period, in seconds 
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

    std::string WinAPI::saveScreenshot() 
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

        if (hBitmap == nullptr) {
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

        // Generate a unique file name using the current time
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        tm timeInfo;
        localtime_s(&timeInfo, &in_time_t);
        ss << "./image/screenshot_" << std::put_time(&timeInfo, "%Y%m%d%H%M%S") << ".png";
        std::string fileName = ss.str();

        std::fstream fileBuffer;
        fileBuffer.open(fileName, std::fstream::binary | std::fstream::out);
        if (!fileBuffer.is_open()) {
            std::cout << "Failed to open file for writing.\n";
            DeleteObject(hBitmap);
            DeleteDC(hdcMemDC);
            return "Failed to open file for writing.";
        }

        fileBuffer.write(reinterpret_cast<const char*>(&buf[0]), buf.size() * sizeof(BYTE));
        fileBuffer.close();

        DeleteObject(hBitmap);
        DeleteDC(hdcMemDC);

        return fileName;
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

    bool WinAPI::copyFolder(const wchar_t* sourceFolder, const wchar_t* destinationFolder) 
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
            return false;
        }

        // Create destination folder
        if (!CreateDirectoryW(destPath.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
            FindClose(hFind);
            return false;
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
                if (!copyFolder(sourceFilePath.c_str(), destFilePath.c_str())) {
                    FindClose(hFind);
                    return false;
                }
            } else {
                // Copy file
                if (!CopyFileW(sourceFilePath.c_str(), destFilePath.c_str(), FALSE)) {
                    FindClose(hFind);
                    return false;
                }
            }
        } while (FindNextFileW(hFind, &findFileData) != 0);

        FindClose(hFind);
        return true;
    }

    void WinAPI::printProcessNameAndID(DWORD processID, std::wofstream &file)
    {
        TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

        if (hProcess != NULL) {
            HMODULE hMod;
            DWORD cbNeeded;
            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
                GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
            }
            CloseHandle(hProcess);
        }

        file << szProcessName << L"  (PID: " << processID << L")\n";
    } 
    
    std::string WinAPI::listProcesses(const std::wstring &filename)
    {
        std::wstring result;
        std::wofstream file(filename);
        if (!file.is_open()) {
            result = L"Failed to open file\n";
            std::wcerr << result;
            return WinAPI::wcharToString(result.c_str());
        }

        DWORD aProcesses[1024], cbNeeded, cProcesses;
        if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
            result = L"Failed to enumerate processes\n";
            std::wcerr << result;
            return WinAPI::wcharToString(result.c_str());
        }

        cProcesses = cbNeeded / sizeof(DWORD);
        for (unsigned int i = 0; i < cProcesses; i++)
            if (aProcesses[i] != 0)
                printProcessNameAndID(aProcesses[i], file);

        return WinAPI::wcharToString(L"Successfully list all processes.\n");
    }
