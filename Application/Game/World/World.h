#pragma once

#include <filesystem>

#include "../../Sei/Camera/Camera.h"
#include "../../Sei/AssetLoader/SceneDataLoader.h"

namespace Game::World
{
	bool LoadMap(const std::filesystem::path& path, std::string& error);
	void Draw(const Sei::Camera& camera);
	DirectX::XMFLOAT3 GetSpawnPosition();
	void Clear();
}
