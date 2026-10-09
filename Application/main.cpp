#include "Sei/Window/Window.h"
#include "Sei/Render/Render.h"

#include "Sei/Camera/FreeCameraController.h"
#include "Sei/Input/Input.h"
#include "Sei/Input/Keybinds.h"
#include "Sei/HUD/HUD.h"

#include "Game/World/World.h"

#include <iostream>
#include <chrono>
#include <format>
#include <cmath>

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


	// Keep the initial viewing direction while the camera moves.
	Sei::Camera camera;
	//camera.SetPosition(8.0f, 5.0f, -10.0f);
	//camera.LookAt(0.0f, 1.5f, 0.0f);

	// (temp) spawn on floor
	const auto spawn = Game::World::GetSpawnPosition();
	camera.SetPosition(spawn.x, spawn.y, spawn.z);
	camera.SetRotation(0.0f, 0.0f); // Face +Z.

	RECT client = {};
	GetClientRect(Sei::Window::GetHandle(), &client);
	const float aspect = static_cast<float>(client.right - client.left) /
		static_cast<float>(client.bottom - client.top);
	//camera.SetPerspective(DirectX::XM_PIDIV4, aspect, 0.1f, 100.0f);

	// quake fov
	const float horizontalFov = DirectX::XMConvertToRadians(103.0f);
	const float verticalFov =
		2.0f * std::atan(std::tan(horizontalFov * 0.5f) / aspect);
	camera.SetPerspective(verticalFov, aspect, 0.1f, 100.0f);

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
		Sei::FreeCameraController::Update(camera, deltaTime, 10.0f);

		Sei::Render::BeginFrame();
		Game::World::Draw(camera);
		Sei::Render::EndFrame();
	}

	Game::World::Clear();
	Sei::Render::Shutdown();
	Sei::Window::Destroy();
	Sei::Input::Shutdown();
	Sei::Keybinds::Clear();
	return 0;
}
