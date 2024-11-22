#include "SystemOperations.hpp"

BOOL SystemOperations::systemShutdown()
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

BOOL SystemOperations::systemRestart(LPWSTR lpMsg)
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

void SystemOperations::initializeGDIPlus()
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::Status status = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    if (status == Gdiplus::Ok)
        std::cout << "GDIPlus successfully initialized.\n";
    else
        std::cout << "Failed to initialize GDI+. Status: " << status << "\n";
}

std::string SystemOperations::saveScreenshot(const std::string& filePath) 
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