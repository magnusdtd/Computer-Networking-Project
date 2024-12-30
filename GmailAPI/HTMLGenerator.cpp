#include "HTMLGenerator.hpp"

std::string HTMLGenerator::htmlMail(const std::string &content)
{
    std::ostringstream html;
    html << R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8" />
        <style>
            * { box-sizing: border-box; }
            html { font-family: 'Roboto', sans-serif; }
            p, td, th, span, ul { color: #333; font-size: 16px; }
            .main { margin: 0 auto; border: 1px solid #ccc; border-radius: 10px; padding: 6px 30px 30px 30px; width: 700px; }
            .app__name { text-align: center; font-size: 24px; color:rgb(162, 28, 28); font-weight: bold; }
            .app__greeting, .app__desc { text-align: center; }
            .divider { border-bottom: 1px solid #ccc; margin: 20px 0; }
            .response { font-weight: bold; word-break: break-all; }
            table { margin: 0 auto; border-collapse: collapse; overflow: hidden; }
            table td, table th { font-size: 14px; }
            table.left { text-align: left; }
            table.center { text-align: center; }
            td, th { border-top: 1px solid rgba(24, 21, 191, 0.9); padding: 10px 14px; }
            th { background-color: #76dfd8; border-left: 1px solid #c6cbcd; border-right: 1px solid #c6cbcd; }
            td { border-left: 1px solid #c6cbcd; border-right: 1px solid #c6cbcd; }
            tr.first-row { text-align: center; }
            tr.last-row { border-bottom: 1px solid #c6cccde6; }
            tr.odd-row td { background-color: #e6f8f7; }
            .message { margin: 0; }
            .message.bold { font-weight: bold; }
            .message.ok { color: #1e9d95; }
            .message.error { color: red; }
            .ascii { font-family: 'Courier New', monospace; font-size: 16px; margin: 0; margin-left: 40px; white-space: pre-wrap; font-weight: bold; }
        </style>
    </head>
    <body>
        <div class='main'>
            <div class="container">
                <p class="app__name">👻 Remote Control with Email Service 👻</p>
                <p class="app__greeting">Greeting from <span style='font-weight: bold;'>Đàm Tiến Đạt, Huỳnh Cung, Lê Xuân Trí </span></p>
                <p class="app__greeting">Regular Program 2023, University of Science, VNUHCM</p>
                <p class="app__desc">This is final project for <span style='font-weight: bold;'>📚 Computer Networking 🛜</span> course (CSC10008).</p>
                <div class='divider'></div>
            </div>
            <div class="container">
                <p>Response from server: <span class="response" lang="en">)" << content << R"(</span></p>
            </div>
        </div>
    </body>
    </html>
    )";
    return html.str();
}

std::vector<std::string> HTMLGenerator::readAndCleanCsvFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    std::vector<std::string> lines;
    if (!file.is_open()) {
        lines.push_back("Error: Unable to open file.");
        return lines;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Convert commas to dots for numbers
        bool insideQuotes = false;
        for (size_t i = 0; i < line.size(); ++i) {
            if (line[i] == '\"') {
                insideQuotes = !insideQuotes;
            } else if (insideQuotes && line[i] == ',') {
                line[i] = '.'; // Convert comma to dot
            }
        }
        // Remove double quotes
        line.erase(std::remove(line.begin(), line.end(), '\"'), line.end());
        lines.push_back(line);
    }

    file.close();
    return lines;
}

std::string HTMLGenerator::csvToHtmlTable(const std::string& filePath)
{
    std::vector<std::string> lines = readAndCleanCsvFile(filePath);
    if (lines.empty() || lines[0] == "Error: Unable to open file.") {
        return lines[0];
    }

    std::ostringstream html;
    html << "<table class='center' style='border-collapse: collapse; border: 1px solid black;'>";

    bool isHeader = true;
    for (const auto& line : lines) {
        html << "<tr>";
        std::istringstream lineStream(line);
        std::string cell;
        while (std::getline(lineStream, cell, ',')) {
            if (isHeader) {
                html << "<th style='border: 1px solid black;'>" << cell << "</th>";
            } else {
                html << "<td style='border: 1px solid black;'>" << cell << "</td>";
            }
        }
        html << "</tr>";
        isHeader = false;
    }

    html << "</table>";
    return html.str();
}