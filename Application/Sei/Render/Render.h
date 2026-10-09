#pragma once
#include <Windows.h>

#include "Mesh.h"
#include "MeshData.h"
#include "Shader.h"
#include "../Camera/Camera.h"
#include <filesystem>
#include <string>
#include <DirectXMath.h>

namespace Sei::Render
{
	bool Initialize(HWND window);
	void LogDebug();
	void BeginFrame();
	void ToggleWireframe();
	void EndFrame();
	void Draw(const Mesh& mesh, const Shader& shader, const DirectX::XMMATRIX& worldViewProjection,
		const DirectX::XMMATRIX& world = DirectX::XMMatrixIdentity());
	void Draw(const Mesh& mesh, const Shader& shader, const Camera& camera);
	void Draw(const Mesh& mesh, const Shader& shader, const Camera& camera, const DirectX::XMFLOAT4X4& transform);
	void Shutdown();

	bool CreateMesh(const MeshData& data, Mesh& output);
	bool CreateShader(const std::filesystem::path& path, Shader& output, std::string& error);
}
