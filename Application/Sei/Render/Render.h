#pragma once
#include <Windows.h>

#include "Mesh.h"
#include "MeshData.h"
#include "Shader.h"
#include "../Camera/Camera.h"
#include <filesystem>
#include <string>
#include <DirectXMath.h>
#include <vector>

namespace Sei::HUD { struct Section; }

namespace Sei::Render
{
	struct RenderItem
	{
		const Mesh* mesh;
		const Shader* shader;
		DirectX::XMFLOAT4X4 transform;
	};
	struct RenderQueue
	{
		const Camera* camera = nullptr;
		std::vector<RenderItem> items;
		struct BoxItem { DirectX::XMFLOAT3 min, max; };
		std::vector<BoxItem> boxes;
		std::vector<const HUD::Section*> hudSections;
	};

	bool Initialize(RenderQueue& renderQueue, HWND window);
	IDXGISurface* GetBackBufferSurface();
	void LogDebug();
	void BeginFrame();
	void ToggleWireframe();
	void EndFrame();
	void Draw(const Mesh& mesh, const Shader& shader, const DirectX::XMMATRIX& worldViewProjection,
		const DirectX::XMMATRIX& world = DirectX::XMMatrixIdentity(),
		D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	void DrawBox(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max, const Camera& camera);
	void Draw(const Mesh& mesh, const Shader& shader, const Camera& camera);
	void Draw(const Mesh& mesh, const Shader& shader, const Camera& camera, const DirectX::XMFLOAT4X4& transform);
	void Shutdown();

	bool CreateMesh(const MeshData& data, Mesh& output);
	bool CreateShader(const std::filesystem::path& path, Shader& output, std::string& error);

	void RenderAll();
}
