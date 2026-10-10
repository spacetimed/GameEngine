#include <filesystem>
#include <string>
#include <iostream>

#include "../../Sei/Camera/Camera.h"
#include "../../Sei/AssetLoader/SceneDataLoader.h"

#include "World.h"
#include "../../Sei/Render/Render.h"
#include <utility>
#include <unordered_set>

namespace Game::World
{
	namespace
	{
		Sei::SceneDataLoader::SceneData WorldScene;
		Sei::Shader worldShader;
		std::unordered_set<std::string> overlappingObjects;
	}

	bool CreateWorldFromMap(const std::filesystem::path& path)
	{
		std::string error;

		// load map scene objects into vertex/index buffers
		Sei::SceneDataLoader::SceneData loaded;
		if (!Sei::SceneDataLoader::loadSceneData(path, loaded, error))
		{
			std::cerr << "scene loading failed: " + error;
			return 0;
		}

		// All objects currently use the same basic lighting shader.
		Sei::Shader shader;
		if (!Sei::Render::CreateShader("Resources/Shaders/Solid.hlsl", shader, error))
			return false;
		WorldScene = std::move(loaded);
		worldShader = std::move(shader);
		overlappingObjects.clear();

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
		return { t._41, t._42 + 2.0f, t._43 - 6.0f }; // Spawn away from the central pillar.
	}

	Sei::Player::Collision::Hit sweep(const Sei::Player::Collision::AABB& playerBox,
		const DirectX::XMFLOAT3& displacement)
	{
		Sei::Player::Collision::Hit closest;
		for (const auto& [name, object] : WorldScene.objects)
		{
			const auto box = Sei::Player::Collision::transform(
				{ object.mesh.boundsMin, object.mesh.boundsMax }, object.transform);
			const auto hit = Sei::Player::Collision::sweep(playerBox, displacement, box);
			if (hit.hit && (!closest.hit || hit.fraction < closest.fraction)) closest = hit;
		}
		return closest;
	}

	void checkOverlaps(const Sei::Player::Collision::AABB& playerBox)
	{
		for (const auto& [name, object] : WorldScene.objects)
		{
			const auto box = Sei::Player::Collision::transform(
				{ object.mesh.boundsMin, object.mesh.boundsMax }, object.transform);
			if (Sei::Player::Collision::overlap(playerBox, box))
			{
				if (overlappingObjects.insert(name).second)
					std::cout << "Player overlaps " << name << '\n';
			}
			else overlappingObjects.erase(name);
		}
	}

	void Clear()
	{
		WorldScene.objects.clear();
		worldShader = {};
		overlappingObjects.clear();
	}
}
