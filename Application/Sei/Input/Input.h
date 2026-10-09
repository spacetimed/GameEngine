#pragma once
#include <Windows.h>

// handles input layer of application (which keys are held/pressed, mouse deltas, cursor capture)

namespace Sei::Input
{
    inline float mouseSensitivity = 0.002f; // Radians per raw mouse count.
    bool Initialize(HWND window);
    void Shutdown();
    void ProcessMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    void Update();
    bool KeyDown(unsigned int key);
    bool KeyPressed(unsigned int key);
    float GetMouseDeltaX();
    float GetMouseDeltaY();
    bool IsMouseCaptured();
    void CaptureMouse();
    void ReleaseMouse();
}
