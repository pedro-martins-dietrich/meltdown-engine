#pragma once

#include "../Utils/EngineStructs.hpp"

namespace mtd::MeshIO
{
	// Saves mesh data to a `.mesh` file with the specified name, returning true if successful
	bool saveMesh
	(
		std::string_view fileName,
		const std::vector<Vertex>& vertices,
		const std::vector<uint32_t>& indices,
		const std::vector<SubmeshData>& submeshData
	);

	// Loads mesh data from a `.mesh` file in the specified path, returning true if successful
	bool loadMesh
	(
		std::string_view fileName,
		std::vector<Vertex>& vertices,
		std::vector<uint32_t>& indices,
		std::vector<SubmeshData>& submeshData
	);
}
