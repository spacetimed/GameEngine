#include "Sei/Window/Window.h"
#include "Sei/Render/Render.h"

#include "Sei/AssetLoader/SceneDataLoader.h"
#include "Sei/Camera/FreeCameraController.h"
#include "Sei/Input/Input.h"
#include "Sei/Input/Keybinds.h"
#include "Sei/HUD/HUD.h"
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

	// Select the arena objects by name.
	const auto& floor = scene.objects.at("Floor");
	const auto& pillar1 = scene.objects.at("Pillar1");
	const auto& pillar2 = scene.objects.at("Pillar2");
	const auto& step = scene.objects.at("Step");
	const auto& step1 = scene.objects.at("Step.001");
	const auto& step2 = scene.objects.at("Step.002");
	const auto& step3 = scene.objects.at("Step.003");

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
	//camera.SetPosition(8.0f, 5.0f, -10.0f);
	//camera.LookAt(0.0f, 1.5f, 0.0f);

	// (temp) spawn on floor
	const auto& t = floor.transform;
	camera.SetPosition(
		t._41,         // Floor's X position.
		t._42 + 2.0f,  // Two units above its origin.
		t._43);        // Floor's Z position.
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
		Sei::Render::Shutdown();
		Sei::Window::Destroy();
		return 1;
	}
	std::cout << "Click to look around; Escape releases the mouse.\n";
	auto previousTime = std::chrono::steady_clock::now();

	// HUD section reads the current value of this string each frame.
	std::string currFps = "...";
	Sei::HUD::Section fpsOverlay;
	fpsOverlay.items.push_back({ "FPS", &currFps });
	Sei::HUD::BindSection(fpsOverlay);

	Sei::Keybinds::Bind(VK_F2, [] {
		Sei::Render::ToggleWireframe();
	});
	float fpsElapsed = 0.0f;
	unsigned int fpsFrames = 0;

	// main loop
	while (Sei::Window::Listen())
	{
		// Seconds elapsed since the previous frame.
		const auto now = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration<float>(now - previousTime).count();
		previousTime = now;

		// Average FPS over half a second, using actual elapsed time.
		fpsElapsed += deltaTime;
		++fpsFrames;
		if (fpsElapsed >= 0.5f)
		{
			currFps = std::to_string(static_cast<unsigned int>(fpsFrames / fpsElapsed + 0.5f));
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
		Sei::Render::Draw(floor.mesh, solidShader, camera, floor.transform);
		Sei::Render::Draw(pillar1.mesh, solidShader, camera, pillar1.transform);
		Sei::Render::Draw(pillar2.mesh, solidShader, camera, pillar2.transform);
		Sei::Render::Draw(step.mesh, solidShader, camera, step.transform);
		Sei::Render::Draw(step1.mesh, solidShader, camera, step1.transform);
		Sei::Render::Draw(step2.mesh, solidShader, camera, step2.transform);
		Sei::Render::Draw(step3.mesh, solidShader, camera, step3.transform);
		Sei::Render::EndFrame();
	}

	Sei::Render::Shutdown();
	Sei::Window::Destroy();
	Sei::Input::Shutdown();
	Sei::Keybinds::Clear();
	return 0;
}
