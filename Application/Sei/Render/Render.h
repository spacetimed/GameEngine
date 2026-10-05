#pragma once
#include <Windows.h>

namespace Sei::Render
{
	bool Initialize(HWND window);
	void LogDebug();
	void BeginFrame();
	void EndFrame();
	void Shutdown();
}