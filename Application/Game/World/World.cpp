#include <filesystem>
#include <string>
#include <iostream>

#include "../../Sei/Camera/Camera.h"
#include "../../Sei/AssetLoader/SceneDataLoader.h"

#include "World.h"
#include "../../Sei/Render/Render.h"
#include <utility>

namespace Game::World
{
	namespace
	{
		Sei::SceneDataLoader::SceneData WorldScene;
		Sei::Shader worldShader;
	}

	bool LoadMap(const std::filesystem::path& path, std::string& error)
	{
		error.clear();

		// load map scene objects into vertex/index buffers
		Sei::SceneDataLoader::SceneData loaded;
		if (!Sei::SceneDataLoader::loadSceneData(path, loaded, error))
		{
			error = "scene loading failed: " + error; // propagate errors
			return false;
		}
		// All objects currently use the same basic lighting shader.
		Sei::Shader shader;
		if (!Sei::Render::CreateShader("Resources/Shaders/Solid.hlsl", shader, error))
			return false;
		WorldScene = std::move(loaded);
		worldShader = std::move(shader);

		std::cout << "world scene loaded!: " << WorldScene.objects.size() << " objects\n";

		for (const auto& [name, object] : WorldScene.objects)
			std::cout << "  " << name << ": " << object.mesh.indexCount / 3 << " triangles\n";

		return true;
	}

	void Draw(const Sei::Camera& camera)
	{
		for (const auto& [name, object] : WorldScene.objects)
			Sei::Render::Draw(object.mesh, worldShader, camera, object.transform);
	}

	DirectX::XMFLOAT3 GetSpawnPosition()
	{
		const auto floor = WorldScene.objects.find("Floor");
		if (floor == WorldScene.objects.end()) return { 0.0f, 2.0f, 0.0f };
		const auto& t = floor->second.transform;
		return { t._41, t._42 + 2.0f, t._43 };
	}

	void Clear()
	{
		WorldScene.objects.clear();
		worldShader = {};
	}
}
