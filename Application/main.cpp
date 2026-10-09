#include "Sei/Window/Window.h"
#include "Sei/Render/Render.h"

#include "Sei/Input/Input.h"
#include "Sei/Input/Keybinds.h"
#include "Sei/HUD/HUD.h"

#include "Game/World/World.h"
#include "Game/Player/Player.h"

#include <iostream>
#include <chrono>
#include <format>

int main()
{
	std::string error;

	// create window; ensure success
	if (!Sei::Window::Create()) return 1;

	// initialize renderer; ensure success
	if (!Sei::Render::Initialize(Sei::Window::GetHandle()))
	{
		Sei::Render::Shutdown();
		Sei::Window::Destroy();
		return 1;
	}

	if (!Game::World::LoadMap("Resources/Maps/arena.glb", error))
	{
		std::cerr << error << '\n';
		Sei::Render::Shutdown();
		Sei::Window::Destroy();
		return 1;
	}


	// (temp) spawn on floor
	const auto spawn = Game::World::GetSpawnPosition();
	Game::Player::spawn(spawn.x, spawn.y, spawn.z);

	RECT client = {};
	GetClientRect(Sei::Window::GetHandle(), &client);

	if (!Sei::Input::Initialize(Sei::Window::GetHandle()))
	{
		std::cerr << "Could not initialize raw mouse input.\n";
		Sei::Input::Shutdown();
		Game::World::Clear();
		Sei::Render::Shutdown();
		Sei::Window::Destroy();
		return 1;
	}
	auto previousTime = std::chrono::steady_clock::now();

	// HUD section reads the current value of this string each frame.
	std::string currFps = "0";
	std::string currFrametime = "0.00 ms";
	std::string resolution = std::format("{}x{}", client.right - client.left, client.bottom - client.top);

	Sei::HUD::Section fpsOverlay;

	fpsOverlay.items.push_back({ "fps", &currFps });
	fpsOverlay.items.push_back({ "frametime", &currFrametime });
	fpsOverlay.items.push_back({ "resolution", &resolution });

	// bind all hud elements onto renderer (todo: refactor all rendering to render.cpp)
	Sei::HUD::BindSection(fpsOverlay);

	// keybinds
	Sei::Keybinds::Bind(VK_F2, Sei::Render::ToggleWireframe);
	Sei::Keybinds::Bind(VK_OEM_3, [&fpsOverlay] { fpsOverlay.ToggleVisibility(); });

	float fpsElapsed = 0.0f;
	unsigned int fpsFrames = 0;

	// main loop
	while (Sei::Window::Listen())
	{
		// Seconds elapsed since the previous frame.
		const auto now = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration<float>(now - previousTime).count();
		previousTime = now;

		// Average FPS and frametime over half a second, using actual elapsed time.
		fpsElapsed += deltaTime;
		++fpsFrames;
		if (fpsElapsed >= 0.5f)
		{
			currFps = std::to_string(static_cast<unsigned int>(fpsFrames / fpsElapsed + 0.5f));
			currFrametime = std::format("{:.2f} ms", fpsElapsed * 1000.0f / fpsFrames);
			fpsElapsed = 0.0f;
			fpsFrames = 0;
		}

		// Avoid a large movement jump after a pause.
		if (deltaTime > 0.1f)
			deltaTime = 0.1f;

		Sei::Input::Update();
		Sei::Keybinds::Update();
		Game::Player::update(deltaTime);

		Sei::Render::BeginFrame();
		Game::World::Draw(Game::Player::getCamera());
		Sei::Render::EndFrame();
	}

	Game::World::Clear();
	Sei::Render::Shutdown();
	Sei::Window::Destroy();
	Sei::Input::Shutdown();
	Sei::Keybinds::Clear();
	return 0;
}
