#include "SceneDataLoader.h"
#include "../Render/Render.h"

#define CGLTF_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable: 4996) // cgltf uses portable C runtime functions.
#include "../../ThirdParty/cgltf/cgltf.h"
#pragma warning(pop)

#include <limits>
#include <memory>
#include <utility>

namespace Sei::SceneDataLoader
{
    bool loadSceneData(
        const std::filesystem::path& path,
        SceneData& res,
        std::string& error
    )
    {
        error.clear();

        // load file and its geometry buffers
        cgltf_options options = {};
        cgltf_data* parsed = nullptr;
        const std::string filename = path.string();
        if (cgltf_parse_file(&options, filename.c_str(), &parsed) != cgltf_result_success)
        {
            error = "failed to parse SceneData: " + filename;
            return false;
        }

        // Free the parsed file on both successful and failed returns.
        std::unique_ptr<cgltf_data, decltype(&cgltf_free)> data(parsed, cgltf_free);
        if (cgltf_load_buffers(&options, data.get(), filename.c_str()) != cgltf_result_success ||
            cgltf_validate(data.get()) != cgltf_result_success)
        {
            error = "failed to load or validate scene buffers: " + filename;
            return false;
        }

        const cgltf_scene* scene = data->scene;
        if (!scene && data->scenes_count > 0)
            scene = &data->scenes[0];
        if (!scene)
        {
            error = "file contains no scene";
            return false;
        }

        SceneData loaded;
        std::unordered_map<const cgltf_mesh*, Mesh> uploadedMeshes;

        auto visit = [&](auto&& self, const cgltf_node& node) -> bool
        {
            if (node.mesh)
            {
                if (node.skin || node.mesh->weights_count)
                {
                    error = "skinned or morphed objects are not supported yet";
                    return false;
                }

                // Unnamed nodes get a stable fallback name; duplicate names are errors.
                const std::string name = node.name && node.name[0] ? node.name :
                    "Object_" + std::to_string(&node - data->nodes);
                if (loaded.objects.contains(name))
                {
                    error = "duplicate scene object name: " + name;
                    return false;
                }

                auto cached = uploadedMeshes.find(node.mesh);
                if (cached == uploadedMeshes.end())
                {
                    MeshData geometry;
                    for (cgltf_size p = 0; p < node.mesh->primitives_count; ++p)
                    {
                        const cgltf_primitive& primitive = node.mesh->primitives[p];
                        if (primitive.type != cgltf_primitive_type_triangles || primitive.targets_count)
                        {
                            error = "only static triangle geometry is supported: " + name;
                            return false;
                        }

                        const cgltf_accessor* positions = nullptr;
                        const cgltf_accessor* normals = nullptr;
                        for (cgltf_size a = 0; a < primitive.attributes_count; ++a)
                        {
                            if (primitive.attributes[a].type == cgltf_attribute_type_position)
                                positions = primitive.attributes[a].data;
                            if (primitive.attributes[a].type == cgltf_attribute_type_normal)
                                normals = primitive.attributes[a].data;
                        }

                        if (!positions || positions->type != cgltf_type_vec3 || !positions->count)
                        {
                            error = "mesh has no valid positions: " + name;
                            return false;
                        }

                        if (!normals || normals->type != cgltf_type_vec3 || normals->count != positions->count)
                        {
                            error = "mesh has no valid normals: " + name;
                            return false;
                        }

                        const auto maxBufferBytes = (std::numeric_limits<UINT>::max)();
                        if (positions->count > maxBufferBytes / sizeof(Vertex) - geometry.vertices.size())
                        {
                            error = "vertex buffer is too large: " + name;
                            return false;
                        }
                        const auto baseVertex = static_cast<std::uint32_t>(geometry.vertices.size());
                        for (cgltf_size v = 0; v < positions->count; ++v)
                        {
                            float xyz[3];
                            float normal[3];
                            if (!cgltf_accessor_read_float(positions, v, xyz, 3) ||
                                !cgltf_accessor_read_float(normals, v, normal, 3))
                            {
                                error = "could not read vertex or normal: " + name;
                                return false;
                            }
                            // glTF is right-handed; Sei currently uses left-handed coordinates.
                            geometry.vertices.push_back({
                                xyz[0], xyz[1], -xyz[2],
                                normal[0], normal[1], -normal[2]
                            });
                        }

                        const cgltf_size count = primitive.indices ? primitive.indices->count : positions->count;
                        if (!count || count % 3 ||
                            count > maxBufferBytes / sizeof(std::uint32_t) - geometry.indices.size())
                        {
                            error = "invalid triangle indices: " + name;
                            return false;
                        }
                        for (cgltf_size i = 0; i < count; i += 3)
                        {
                            std::uint32_t triangle[3];
                            for (cgltf_size corner = 0; corner < 3; ++corner)
                            {
                                const cgltf_size index = primitive.indices ?
                                    cgltf_accessor_read_index(primitive.indices, i + corner) : i + corner;
                                if (index >= positions->count)
                                {
                                    error = "index references an invalid vertex: " + name;
                                    return false;
                                }
                                triangle[corner] = baseVertex + static_cast<std::uint32_t>(index);
                            }
                            // Reverse winding to match the handedness conversion.
                            geometry.indices.insert(geometry.indices.end(),
                                { triangle[0], triangle[2], triangle[1] });
                        }
                    }

                    Mesh mesh;
                    if (!Render::CreateMesh(geometry, mesh))
                    {
                        error = "could not upload mesh (initialize Render first): " + name;
                        return false;
                    }
                    cached = uploadedMeshes.emplace(node.mesh, std::move(mesh)).first;
                }

                SceneObject object;
                object.name = name;
                object.mesh = cached->second; // ComPtr copies share the existing GPU buffers.
                float matrix[16];
                cgltf_node_transform_world(&node, matrix); // Includes parent transforms.
                const DirectX::XMFLOAT4X4 gltfTransform(matrix);
                const auto flipZ = DirectX::XMMatrixScaling(1.0f, 1.0f, -1.0f);
                DirectX::XMStoreFloat4x4(&object.transform,
                    flipZ * DirectX::XMLoadFloat4x4(&gltfTransform) * flipZ);
                loaded.objects.emplace(name, std::move(object));
            }

            for (cgltf_size child = 0; child < node.children_count; ++child)
                if (!self(self, *node.children[child])) return false;
            return true;
        };

        for (cgltf_size root = 0; root < scene->nodes_count; ++root)
            if (!visit(visit, *scene->nodes[root])) return false;

        if (loaded.objects.empty())
        {
            error = "scene contains no mesh objects";
            return false;
        }
        res = std::move(loaded);
        return true;
    }
}
