#include "Sei/Window/Window.h"
#include "Sei/Render/Render.h"

int main()
{
	// create window; ensure success
	if (!Sei::Window::Create()) return 1;

	// initialize renderer; ensure success
	if (!Sei::Render::Initialize(Sei::Window::GetHandle()))
	{
		Sei::Render::Shutdown();
		Sei::Window::Destroy();
		return 1;
	}

	// main loop
	while (Sei::Window::Listen())
	{
		Sei::Render::BeginFrame();
		Sei::Render::EndFrame();
	}

	Sei::Render::Shutdown();
	Sei::Window::Destroy();
	return 0;
}