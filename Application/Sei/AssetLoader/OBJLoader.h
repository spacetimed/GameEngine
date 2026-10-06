#pragma once

#include "../Render/MeshData.h"
#include <filesystem>
#include <string>

namespace Sei::AssetLoader
{
    bool LoadOBJ(const std::filesystem::path& path, MeshData& output, std::string& error);
}