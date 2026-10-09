#pragma once

// load a .glb, and for each unique object:
//  1. create MeshData (vertices, indices)
//  2. load mesh into vertex/index buffers on GPU
//  3. return set of objects

// returns: SceneData: [SceneObject, ...]
//  SceneObject: [name, mesh reference, imported transform]

#include <string>
#include <DirectXMath.h>
#include <unordered_map>
#include <filesystem>

#include "../Render/Mesh.h"

namespace Sei::SceneDataLoader
{
    struct SceneObject
    {
        std::string name;
        Mesh mesh;
        DirectX::XMFLOAT4X4 transform;
    };

    struct SceneData
    {
        std::unordered_map<std::string, SceneObject> objects;
    };

    bool loadSceneData(
        const std::filesystem::path& path,
        SceneData& res,
        std::string& error
    );
}