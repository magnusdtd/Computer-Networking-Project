#ifndef MY_UTILITY_HPP
#define MY_UTILITY_HPP

#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <vector>
#include <windows.h>

namespace MyUtility {
    std::string generateName(const std::string& prefixName, const std::string& extensionName);
    std::string wcharToString(const wchar_t* wstr);
};


#endif