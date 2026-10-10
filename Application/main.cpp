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
#include "Sei/Performance/Performance.h"

bool WorldInitialized = false;
bool PlayerInitialized = false;
bool InputInitialized = false;

int epilogue(int exitCode = 1)
{
	if (InputInitialized)	Sei::Input::Shutdown();
	if (WorldInitialized)	Game::World::Shutdown();
	if (PlayerInitialized)	Game::Player::Shutdown();
	Sei::Render::Shutdown();
	Sei::Window::Shutdown();
	return exitCode;
}

int main()
{
	if (!Sei::Window::Create()) return 1;
	if (!Sei::Render::Initialize(Sei::Window::GetHandle())) return epilogue();

	// start world
	WorldInitialized = true;
	if (!Game::World::CreateWorldFromMap("Resources/Maps/arena.glb")) return epilogue();

	// start player
	PlayerInitialized = true;
	const auto spawn = Game::World::GetSpawnPosition();
	if (!Game::Player::spawn(spawn.x, spawn.y, spawn.z)) return epilogue();

	// calc res; start performance monitor
	RECT client = {};
	GetClientRect(Sei::Window::GetHandle(), &client);
	Sei::Performance::Init(client.right - client.left, client.bottom - client.top);

	// start input
	InputInitialized = true;
	if (!Sei::Input::Initialize(Sei::Window::GetHandle())) return epilogue();

	// hud overlay
	Sei::HUD::Section perfOverlay;
	perfOverlay.items.push_back({"fps", &Sei::Performance::currFps});
	perfOverlay.items.push_back({"frametime", &Sei::Performance::currFrametime});
	perfOverlay.items.push_back({"res", &Sei::Performance::resolution});
	perfOverlay.items.push_back({"view", &Game::Player::View::mode});
	Sei::HUD::BindToRender(perfOverlay);

	// keybinds
	Sei::Keybinds::Bind(VK_F2, Sei::Render::ToggleWireframe);
	Sei::Keybinds::Bind(VK_F3, Game::Player::View::toggle);
	Sei::Keybinds::Bind(VK_OEM_3, [&perfOverlay] { perfOverlay.ToggleVisibility(); });

	// main loop
	while (Sei::Window::Listen())
	{
		Sei::Performance::tick();

		// process input, keybinds, player
		Sei::Input::Update();
		Sei::Keybinds::Update();
		Game::Player::Update();

		// draw
		Sei::Render::Start();
		Sei::Render::RenderAll();
		Sei::Render::End();
	}

	return epilogue(1);
}
