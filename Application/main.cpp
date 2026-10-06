#include "Sei/Window/Window.h"
#include "Sei/Render/Render.h"

#include "Sei/AssetLoader/OBJLoader.h"
#include "Sei/Camera/FreeCameraController.h"
#include "Sei/Input/Input.h"
#include <iostream>

#include <chrono>

int main()
{
	// smoke test: try to load teapot obj
	Sei::MeshData teapot;
	std::string error;
	if (!Sei::AssetLoader::LoadOBJ("Resources/Models/teapot.obj", teapot, error))
	{
		std::cerr << "model loading failed: " << error << '\n';
		return 1;
	}
	std::cout << "teapot: " << teapot.vertices.size() << " vertices, " << teapot.indices.size() / 3 << " triangles\n";

	// create window; ensure success
	if (!Sei::Window::Create()) return 1;

	// initialize renderer; ensure success
	if (!Sei::Render::Initialize(Sei::Window::GetHandle()))
	{
		Sei::Render::Shutdown();
		Sei::Window::Destroy();
		return 1;
	}

	// try to load teapot mesh
	Sei::Mesh teapotMesh;
	if (!Sei::Render::CreateMesh(teapot, teapotMesh))
	{
		std::cerr << "Could not create teapot GPU buffers.\n";
		Sei::Render::Shutdown();
		Sei::Window::Destroy();
		return 1;
	}
	std::cout << "Teapot uploaded to GPU.\n";

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
		Sei::Render::Draw(teapotMesh, solidShader, camera);
		Sei::Render::EndFrame();
	}

	Sei::Render::Shutdown();
	Sei::Window::Destroy();
	Sei::Input::Shutdown();
	return 0;
}
