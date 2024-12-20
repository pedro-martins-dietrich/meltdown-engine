#pragma once

#include "../../Utils/EngineStructs.hpp"

// Responsible for loading Wavefront .obj files
namespace mtd::ObjMeshLoader
{
	// Loads a default 3D mesh from an Wavefront file
	void loadDefault3DMesh
	(
		const char* fileName,
		std::vector<Vertex>& vertices,
		std::vector<uint32_t>& indices,
		std::string& diffuseTexturePath
	);

	// Loads a 3D mesh with multiple materials from an Wavefront file
	void loadMultiMaterial3DMesh
	(
		const char* fileName,
		std::vector<Vertex>& vertices,
		std::vector<uint32_t>& indices,
		std::vector<SubmeshData>& submeshInfos,
		std::vector<std::string>& texturePaths
	);
}
