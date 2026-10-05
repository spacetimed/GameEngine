#pragma once
#include <windows.h>

namespace Sei::Window
{
    bool Create();
    bool Listen();
    HWND GetHandle();
    void Destroy();
}