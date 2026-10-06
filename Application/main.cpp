#include "Sei/Window/Window.h"
#include "Sei/Render/Render.h"

#include "Sei/AssetLoader/OBJLoader.h"
#include <iostream>

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

	// Fixed camera: view the model from above and to one side.
	using namespace DirectX;
	const XMMATRIX world = XMMatrixIdentity();
	const XMMATRIX view = XMMatrixLookAtLH(
		XMVectorSet(8.0f, 5.0f, -10.0f, 1.0f),
		XMVectorSet(0.0f, 1.5f, 0.0f, 1.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	RECT client = {};
	GetClientRect(Sei::Window::GetHandle(), &client);
	const float aspect = static_cast<float>(client.right - client.left) /
		static_cast<float>(client.bottom - client.top);
	const XMMATRIX projection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f), aspect, 0.1f, 100.0f);
	const XMMATRIX worldViewProjection = world * view * projection;

	// main loop
	while (Sei::Window::Listen())
	{
		Sei::Render::BeginFrame();
		Sei::Render::Draw(teapotMesh, solidShader, worldViewProjection);
		Sei::Render::EndFrame();
	}

	Sei::Render::Shutdown();
	Sei::Window::Destroy();
	return 0;
}
