#include "SystemOperations.hpp"

SystemOperations::SystemOperations()
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::Status status = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    if (status == Gdiplus::Ok)
        std::cout << "GDIPlus successfully initialized.\n";
    else
        std::cout << "Failed to initialize GDI+. Status: " << status << "\n";
}

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

    Gdiplus::Bitmap bitmap(hBitmap, nullptr);
    CLSID pngClsid;
    Gdiplus::Status status = Gdiplus::Ok;

    // Get the CLSID of the PNG encoder.
    int encoderStatus = GetEncoderClsid(L"image/png", &pngClsid);
    if (encoderStatus != 0) {
        std::cout << "Failed to get PNG encoder CLSID.\n";
        DeleteObject(hBitmap);
        DeleteDC(hdcMemDC);
        return "Failed to get PNG encoder CLSID.";
    }

    std::wstring wFilePath(filePath.begin(), filePath.end());
    status = bitmap.Save(wFilePath.c_str(), &pngClsid, nullptr);

    DeleteObject(hBitmap);
    DeleteDC(hdcMemDC);

    if (status != Gdiplus::Ok) {
        std::cout << "Failed to save image to file.\n";
        return "Failed to save image to file.";
    }

    return "Capture screen successful, new file at " + filePath;
}

int SystemOperations::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT num = 0;          // number of image encoders
    UINT size = 0;         // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo* pImageCodecInfo = nullptr;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) {
        return -1;  // Failure
    }

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == nullptr) {
        return -1;  // Failure
    }

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}