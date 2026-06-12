#pragma once
#include <vector>
#include <Windows.h>

class Keyboard {
private:
    static const UINT KEY_COUNT = 256;
    static std::vector<bool> prevKeyState;

public:
    static bool isKeyPressed(UINT keyCode) {
        bool currentState = (GetAsyncKeyState(keyCode) < 0);
        bool prevState = prevKeyState[keyCode];
        prevKeyState[keyCode] = currentState;
        return currentState && !prevState;
    }
    static bool isKeyDown(UINT keyCode) {
        bool currentState = (GetAsyncKeyState(keyCode) < 0);
        prevKeyState[keyCode] = currentState;
        return currentState;
    }
    static bool isKeyUp(UINT keyCode) {
        bool currentState = (GetAsyncKeyState(keyCode) < 0);
        prevKeyState[keyCode] = currentState;
        return !currentState;
    }
};
