#include "ProcessOperations.hpp"

std::string ProcessOperations::StartApplication(const std::wstring& applicationPath) {
    HINSTANCE result = ShellExecuteW(NULL, L"open", applicationPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if ((intptr_t)result <= 32) {
        std::wcerr << L"Failed to start application. Error code: " << (intptr_t)result << std::endl;
        return "Failed";
    }
    return "Succeeded start application with path: " + MyUtility::wcharToString(applicationPath.c_str());
}

std::string ProcessOperations::TerminateProcessByID(DWORD processID) {
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

BOOL CALLBACK ProcessOperations::EnumWindowsProc(HWND hwnd, LPARAM lParam) {
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

std::string ProcessOperations::listRunningApp(std::string &filePath)
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

std::string ProcessOperations::listInstalledApp(std::string &filePath)
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
