#ifndef KEYBOARD_DISABLER_HPP
#define KEYBOARD_DISABLER_HPP

#include <thread>
#include <atomic>
#include <Windows.h>
#include <iostream>

class KeyboardDisabler {
public:
    KeyboardDisabler() : isDisabled(false) {}
    ~KeyboardDisabler();

    void disable();
    void enable();
    bool isKeyboardDisabled() const { return isDisabled; }

private:
    std::atomic<bool> isDisabled;
    std::thread disableThread;

    HHOOK hHook;

    void disableKeyboardThread();
    static LRESULT CALLBACK KeyboardDisabler::KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
};

#endif