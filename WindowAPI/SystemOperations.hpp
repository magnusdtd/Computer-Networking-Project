#ifndef SYSTEM_OPERATIONS_HPP
#define SYSTEM_OPERATIONS_HPP

#include <windows.h>
#include <iostream>
#include <string>
#include <gdiplus.h>
#include <vector>
#include <windows.h>
#include <fstream>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "gdiplus.lib")

class SystemOperations {
public:
    SystemOperations();

    BOOL systemShutdown();
    BOOL systemRestart(LPWSTR lpMsg);
    std::string saveScreenshot(const std::string& filePath);

private:
    int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
};

#endif