#include "MyUtility.hpp"

std::string MyUtility::generateName(const std::string& prefixName, const std::string& extensionName) {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    tm timeInfo;
    localtime_s(&timeInfo, &in_time_t);
    ss << std::put_time(&timeInfo, "%Y%m%d%H%M%S");
    std::string fileName = prefixName + '_' + ss.str() + '.' + extensionName;
    return fileName;
}

std::string MyUtility::wcharToString(const wchar_t* wstr) {
    if (!wstr) return "";

    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (bufferSize == 0)
        return "";

    std::vector<char> buffer(bufferSize);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buffer.data(), bufferSize, nullptr, nullptr);

    return std::string(buffer.data());
}