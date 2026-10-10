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
		Sei::SceneDataLoader::SceneData worldScene;
		Sei::Shader worldShader;
		Sei::Render::RenderQueue* queue = nullptr;
		std::unordered_set<std::string> overlappingObjects;
	}

	bool Initialize(Sei::Render::RenderQueue& renderQueue, const std::filesystem::path& path)
	{
		std::string error;

		// load map scene objects into vertex/index buffers
		Sei::SceneDataLoader::SceneData loaded;
		if (!Sei::SceneDataLoader::loadSceneData(path, loaded, error))
		{
			std::cerr << "scene loading failed: " << error << '\n';
			return false;
		}

		// All objects currently use the same basic lighting shader.
		Sei::Shader shader;
		if (!Sei::Render::CreateShader("Resources/Shaders/Solid.hlsl", shader, error))
		{
			std::cerr << "world shader loading failed: " << error << '\n';
			return false;
		}
		if (queue) queue->items.clear();
		worldScene = std::move(loaded);
		worldShader = std::move(shader);
		overlappingObjects.clear();
		queue = &renderQueue;

		std::cout << "world scene loaded!: " << worldScene.objects.size() << " objects\n";

		for (const auto& [name, object] : worldScene.objects)
			std::cout << "  " << name << ": " << object.mesh.indexCount / 3 << " triangles\n";

		return true;
	}

	void Submit()
	{
		for (const auto& [name, object] : worldScene.objects)
			queue->items.push_back({ &object.mesh, &worldShader, object.transform });
	}

	DirectX::XMFLOAT3 GetSpawnPosition()
	{
		const auto floor = worldScene.objects.find("Floor");
		if (floor == worldScene.objects.end()) return { 0.0f, 2.0f, 0.0f };
		const auto& t = floor->second.transform;
		return { t._41, t._42 + 2.0f, t._43 - 6.0f }; // Spawn away from the central pillar.
	}

	Sei::Player::Collision::Hit Sweep(const Sei::Player::Collision::AABB& playerBox,
		const DirectX::XMFLOAT3& displacement)
	{
		Sei::Player::Collision::Hit closest;
		for (const auto& [name, object] : worldScene.objects)
		{
			const auto box = Sei::Player::Collision::transform(
				{ object.mesh.boundsMin, object.mesh.boundsMax }, object.transform);
			const auto hit = Sei::Player::Collision::sweep(playerBox, displacement, box);
			if (hit.hit && (!closest.hit || hit.fraction < closest.fraction)) closest = hit;
		}
		return closest;
	}

	void CheckOverlaps(const Sei::Player::Collision::AABB& playerBox)
	{
		for (const auto& [name, object] : worldScene.objects)
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

	void Shutdown()
	{
		if (queue) queue->items.clear();
		queue = nullptr;
		worldScene.objects.clear();
		worldShader = {};
		overlappingObjects.clear();
	}
}
