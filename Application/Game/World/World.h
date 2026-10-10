#pragma once

#include <filesystem>

#include "../../Sei/Camera/Camera.h"
#include "../../Sei/AssetLoader/SceneDataLoader.h"
#include "../../Sei/Collision/Collision.h"

namespace Game::World
{
	bool LoadMap(const std::filesystem::path& path, std::string& error);
	void Draw(const Sei::Camera& camera);
	DirectX::XMFLOAT3 GetSpawnPosition();
	void Clear();
	void checkOverlaps(const Sei::Player::Collision::AABB& playerBox);
	Sei::Player::Collision::Hit sweep(const Sei::Player::Collision::AABB& playerBox,
		const DirectX::XMFLOAT3& displacement);
}
