#include "KeyboardDisabler.hpp"

void KeyboardDisabler::disable() {
    if (isDisabled) return;
    isDisabled = true;
    disableThread = std::thread(&KeyboardDisabler::disableKeyboardThread, this);
}

void KeyboardDisabler::enable() {
    if (!isDisabled) return;
    isDisabled = false;
    if (disableThread.joinable()) {
        PostThreadMessage(GetThreadId(disableThread.native_handle()), WM_QUIT, 0, 0);
        disableThread.join();
    }
}

void KeyboardDisabler::disableKeyboardThread() {
    hHook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);
    if (hHook == NULL) {
        std::cerr << "Failed to set keyboard hook. Error: " << GetLastError() << '\n';
        return;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) && isDisabled) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hHook);
    hHook = NULL;
}

LRESULT CALLBACK KeyboardDisabler::KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0) {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    if (wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        return 1; // Block the key press
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}