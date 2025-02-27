#ifndef HTML_GENERATOR_HPP
#define HTML_GENERATOR_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <iomanip>

namespace HTMLGenerator {

    std::string htmlMail(const std::string& content);
    std::string csvToHtmlTable(const std::string& filePath);
    std::vector<std::string> readAndCleanCsvFile(const std::string& filePath);
    
};

#endif