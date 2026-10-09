#pragma once

#include <cstdint>
#include <vector>

namespace Sei
{
	struct Vertex { 
		float x, y, z; 
		float nx, ny, nz;
	};

	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<std::uint32_t> indices;
	};
}