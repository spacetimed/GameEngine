#include "Input.h"

#include <array>

namespace
{
    HWND inputWindow = nullptr;

    std::array<bool, 256> currentKeys = {};
    std::array<bool, 256> previousKeys = {};
    bool mouseCaptured = false;
    int cursorHideCalls = 0;
    float pendingMouseX = 0, pendingMouseY = 0;
    float mouseDeltaX = 0, mouseDeltaY = 0;
}

namespace Sei::Input
{
    bool Initialize(HWND window)
    {
        inputWindow = window;
        currentKeys.fill(false);
        previousKeys.fill(false);
        RAWINPUTDEVICE mouse = {};
        mouse.usUsagePage = 0x01;
        mouse.usUsage = 0x02;
        mouse.hwndTarget = window;
        return RegisterRawInputDevices(&mouse, 1, sizeof(mouse)) != FALSE;
    }

    void CaptureMouse()
    {
        if (mouseCaptured || !inputWindow || GetForegroundWindow() != inputWindow)
            return;
        RECT bounds = {};
        GetClientRect(inputWindow, &bounds);
        if (IsRectEmpty(&bounds)) return;
        POINT topLeft = { bounds.left, bounds.top };
        POINT bottomRight = { bounds.right, bounds.bottom };
        ClientToScreen(inputWindow, &topLeft);
        ClientToScreen(inputWindow, &bottomRight);
        RECT screenBounds = { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
        if (!ClipCursor(&screenBounds)) return;
        SetCapture(inputWindow);
        int count;
        do
        {
            count = ShowCursor(FALSE);
            ++cursorHideCalls;
        } while (count >= 0);
        mouseCaptured = true;
        pendingMouseX = pendingMouseY = mouseDeltaX = mouseDeltaY = 0;
    }

    void ReleaseMouse()
    {
        if (!mouseCaptured) return;
        mouseCaptured = false;
        ClipCursor(nullptr);
        if (GetCapture() == inputWindow) ReleaseCapture();
        while (cursorHideCalls > 0)
        {
            ShowCursor(TRUE);
            --cursorHideCalls;
        }
        pendingMouseX = pendingMouseY = mouseDeltaX = mouseDeltaY = 0;
    }

    void ProcessMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (window != inputWindow) return;
        switch (message)
        {
        case WM_LBUTTONDOWN:
            CaptureMouse();
            break;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) ReleaseMouse();
            break;
        case WM_KILLFOCUS:
        case WM_DESTROY:
        case WM_ENTERSIZEMOVE:
        case WM_SIZE:
            ReleaseMouse();
            break;
        case WM_CAPTURECHANGED:
            if (reinterpret_cast<HWND>(lParam) != inputWindow) ReleaseMouse();
            break;
        case WM_INPUT:
            if (mouseCaptured && GetForegroundWindow() == inputWindow)
            {
                RAWINPUT raw = {};
                UINT size = sizeof(raw);
                const UINT bytes = GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
                    RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));
                if (bytes != static_cast<UINT>(-1) && bytes >= sizeof(RAWINPUTHEADER) + sizeof(RAWMOUSE) &&
                    raw.header.dwType == RIM_TYPEMOUSE && !(raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
                {
                    pendingMouseX += static_cast<float>(raw.data.mouse.lLastX);
                    pendingMouseY += static_cast<float>(raw.data.mouse.lLastY);
                }
            }
            break;
        }
    }

    void Shutdown()
    {
        ReleaseMouse();
        RAWINPUTDEVICE mouse = {};
        mouse.usUsagePage = 0x01;
        mouse.usUsage = 0x02;
        mouse.dwFlags = RIDEV_REMOVE;
        RegisterRawInputDevices(&mouse, 1, sizeof(mouse));
        inputWindow = nullptr;
        currentKeys.fill(false);
        previousKeys.fill(false);
    }

    float GetMouseDeltaX() { return mouseDeltaX; }
    float GetMouseDeltaY() { return mouseDeltaY; }
    bool IsMouseCaptured() { return mouseCaptured; }

    void Update()
    {
        previousKeys = currentKeys;
        currentKeys.fill(false);

        // Publish mouse events accumulated by Window::Listen for this frame.
        mouseDeltaX = pendingMouseX;
        mouseDeltaY = pendingMouseY;
        pendingMouseX = pendingMouseY = 0;
        // Ignore input while another window has focus.
        if (!inputWindow || GetForegroundWindow() != inputWindow)
        {
            ReleaseMouse();
            mouseDeltaX = mouseDeltaY = 0;
            return;
        }

        for (unsigned int key = 0; key < currentKeys.size(); ++key)
        {
            currentKeys[key] =
                (GetAsyncKeyState(key) & 0x8000) != 0;
        }
    }

    bool KeyDown(unsigned int key)
    {
        return key < currentKeys.size() && currentKeys[key];
    }

    bool KeyPressed(unsigned int key)
    {
        return key < currentKeys.size() &&
            currentKeys[key] &&
            !previousKeys[key];
    }
}
