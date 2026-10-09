#include "Sei/Window/Window.h"
#include "Sei/Render/Render.h"

#include "Sei/AssetLoader/SceneDataLoader.h"
#include "Sei/Camera/FreeCameraController.h"
#include "Sei/Input/Input.h"
#include <iostream>

#include <chrono>

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

	// load scene objects into vertex/index buffers
	Sei::SceneDataLoader::SceneData scene;
	if (!Sei::SceneDataLoader::loadSceneData("Resources/Maps/arena.glb", scene, error))
	{
		std::cerr << "Scene loading failed: " << error << '\n';
		Sei::Render::Shutdown();
		Sei::Window::Destroy();
		return 1;
	}
	std::cout << "Arena loaded: " << scene.objects.size() << " objects\n";
	for (const auto& [name, object] : scene.objects)
		std::cout << "  " << name << ": " << object.mesh.indexCount / 3 << " triangles\n";

	// Draw only one named object to verify independent selection.
	const auto selected = scene.objects.find("Pillar2");
	if (selected == scene.objects.end())
	{
		std::cerr << "Arena contains no Pillar2 object.\n";
		Sei::Render::Shutdown();
		Sei::Window::Destroy();
		return 1;
	}
	const auto& pillar = selected->second;

	// load shader into vertex/pixel shader
	Sei::Shader solidShader;
	if (!Sei::Render::CreateShader("Resources/Shaders/Solid.hlsl", solidShader, error))
	{
		std::cerr << "Shader creation failed: " << error << '\n';
		Sei::Render::Shutdown();
		Sei::Window::Destroy();
		return 1;
	}
	std::cout << "Solid shader created.\n";

	// Keep the initial viewing direction while the camera moves.
	Sei::Camera camera;
	camera.SetPosition(8.0f, 5.0f, -10.0f);
	camera.LookAt(0.0f, 1.5f, 0.0f);

	RECT client = {};
	GetClientRect(Sei::Window::GetHandle(), &client);
	const float aspect = static_cast<float>(client.right - client.left) /
		static_cast<float>(client.bottom - client.top);
	camera.SetPerspective(DirectX::XM_PIDIV4, aspect, 0.1f, 100.0f);

	if (!Sei::Input::Initialize(Sei::Window::GetHandle()))
	{
		std::cerr << "Could not initialize raw mouse input.\n";
		Sei::Input::Shutdown();
		Sei::Render::Shutdown();
		Sei::Window::Destroy();
		return 1;
	}
	std::cout << "Click to look around; Escape releases the mouse.\n";
	auto previousTime = std::chrono::steady_clock::now();

	// main loop
	while (Sei::Window::Listen())
	{
		// Seconds elapsed since the previous frame.
		const auto now = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration<float>(now - previousTime).count();
		previousTime = now;

		// Avoid a large movement jump after a pause.
		if (deltaTime > 0.1f)
			deltaTime = 0.1f;

		Sei::Input::Update();
		Sei::FreeCameraController::Update(camera, deltaTime);

		Sei::Render::BeginFrame();
		Sei::Render::Draw(pillar.mesh, solidShader, camera, pillar.transform);
		Sei::Render::EndFrame();
	}

	Sei::Render::Shutdown();
	Sei::Window::Destroy();
	Sei::Input::Shutdown();
	return 0;
}
