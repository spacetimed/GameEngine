#pragma once

#include <filesystem>

#include "../../Sei/Render/Render.h"
#include "../../Sei/AssetLoader/SceneDataLoader.h"
#include "../../Sei/Collision/Collision.h"

namespace Game::World
{
	bool Initialize(Sei::Render::RenderQueue& renderQueue, const std::filesystem::path& path);
	void Submit();
	DirectX::XMFLOAT3 GetSpawnPosition();
	void Shutdown();
	void CheckOverlaps(const Sei::Player::Collision::AABB& playerBox);
	Sei::Player::Collision::Hit Sweep(const Sei::Player::Collision::AABB& playerBox,
		const DirectX::XMFLOAT3& displacement);
}
