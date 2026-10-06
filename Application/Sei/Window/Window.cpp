#include "Window.h"
#include "../Input/Input.h"

namespace Sei::Window
{

    // private
    namespace
    {
        HWND windowHandle = nullptr;
        HINSTANCE windowInstance = nullptr;
        constexpr wchar_t className[] = L"GameWindow";

        LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            Input::ProcessMessage(hwnd, message, wParam, lParam);
            switch (message)
            {
            case WM_DESTROY:
                windowHandle = nullptr;
                PostQuitMessage(0);
                return 0;
            }

            return DefWindowProcW(hwnd, message, wParam, lParam);
        }
    }

    // public
    bool Create()
    {
        HINSTANCE instance = GetModuleHandleW(nullptr);
        int showCommand = SW_SHOWDEFAULT;

        windowInstance = instance;

        WNDCLASSW wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = instance;
        wc.lpszClassName = className;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

        if (!RegisterClassW(&wc)) return false;

        RECT bounds = { 0, 0, 640, 480 };
        if (!AdjustWindowRect(&bounds, WS_OVERLAPPEDWINDOW, FALSE))
        {
            Destroy();
            return false;
        }

        windowHandle = CreateWindowExW(
            0,
            className,
            L"Queue",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            bounds.right - bounds.left,
            bounds.bottom - bounds.top,
            nullptr,
            nullptr,
            instance,
            nullptr
        );

        if (!windowHandle)
        {
            Destroy();
            return false;
        }

        ShowWindow(windowHandle, showCommand);
        return true;
    }

    bool Listen()
    {
        MSG message = {};

        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
                return false;

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        return windowHandle != nullptr;
    }

    HWND GetHandle()
    {
        return windowHandle;
    }

    void Destroy()
    {
        if (windowHandle)
        {
            DestroyWindow(windowHandle);
            windowHandle = nullptr;
        }

        if (windowInstance)
        {
            UnregisterClassW(className, windowInstance);
            windowInstance = nullptr;
        }
    }
}
