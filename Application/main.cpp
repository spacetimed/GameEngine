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
bool KeybindsInitialized = false;
bool HUDInitialized = false;

int Shutdown(int code = 1)
{
	if (InputInitialized)	 Sei::Input::Shutdown();
	if (WorldInitialized)	 Game::World::Shutdown();
	if (PlayerInitialized)	 Game::Player::Shutdown();
	if (KeybindsInitialized) Sei::Keybinds::Shutdown();
	if (HUDInitialized)      Sei::HUD::Shutdown();

	Sei::Render::Shutdown();
	Sei::Window::Shutdown();
	
	return code;
}

int main()
{
	if (!Sei::Window::Create()) return 1;

	// start render
	Sei::Render::RenderQueue renderQueue;
	if (!Sei::Render::Initialize(renderQueue, Sei::Window::GetHandle())) 
		return Shutdown();

	// start world
	WorldInitialized = true;
	if (!Game::World::Initialize(renderQueue, "Resources/Maps/arena.glb"))
		return Shutdown();

	// start player
	PlayerInitialized = true;
	if (!Game::Player::Initialize(renderQueue, Game::World::GetSpawnPosition()))
		return Shutdown();

	// calc res; start performance monitor
	RECT client = {};
	GetClientRect(Sei::Window::GetHandle(), &client);
	Sei::Performance::Initialize(client.right - client.left, client.bottom - client.top);

	// start input
	InputInitialized = true;
	if (!Sei::Input::Initialize(Sei::Window::GetHandle())) 
		return Shutdown();

	// hud
	HUDInitialized = true;
	if (!Sei::HUD::Initialize(renderQueue))
		return Shutdown();

	// create new hud element "perfOverlay"
	Sei::HUD::Section perfOverlay;
	perfOverlay.items.push_back({"fps", &Sei::Performance::currFps});
	perfOverlay.items.push_back({"frametime", &Sei::Performance::currFrametime});
	perfOverlay.items.push_back({"res", &Sei::Performance::resolution});
	perfOverlay.items.push_back({"view", &Game::Player::View::mode});
	Sei::HUD::BindSection(perfOverlay);

	// keybinds
	KeybindsInitialized = true;
	Sei::Keybinds::Bind(VK_F2, Sei::Render::ToggleWireframe);
	Sei::Keybinds::Bind(VK_F3, Game::Player::View::Toggle);
	Sei::Keybinds::Bind(VK_OEM_3, [&perfOverlay] { perfOverlay.ToggleVisibility(); });

	// main loop
	while (Sei::Window::Listen())
	{
		Sei::Performance::tick();

		// process input, keybinds, player
		Sei::Input::Update();
		Sei::Keybinds::Update();
		Game::Player::Update(Sei::Performance::deltaTime);

		// clear render queue, prepare for draw
		Sei::Render::BeginFrame();

		// submit all items into render queue
		Game::World::Submit();
		Game::Player::Submit();
		Sei::HUD::Submit();

		// render all items in render queue
		Sei::Render::RenderAll();

		Sei::Render::EndFrame();
	}

	return Shutdown(0);
}
