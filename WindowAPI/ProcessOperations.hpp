#ifndef PROCESS_OPERATIONS_HPP
#define PROCESS_OPERATIONS_HPP

#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "./../WindowAPI/MyUtility.hpp"

class ProcessOperations {
public:
    std::string StartApplication(const std::wstring& applicationPath);
    std::string TerminateProcessByID(DWORD processID);
    std::string listRunningApp(std::string& filePath);
    std::string listInstalledApp(std::string& filePath);

private:
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

};

#endif