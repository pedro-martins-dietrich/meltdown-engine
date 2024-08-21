#pragma once

#include "../../Utils/EngineStructs.hpp"

// Responsible for loading Wavefront .obj files
namespace mtd::ObjMeshLoader
{
	// Loads a mesh from an Wavefront .obj file
	void load
	(
		const char* fileName,
		std::vector<Vertex>& vertices,
		std::vector<uint32_t>& indices,
		std::string& diffuseTexturePath
	);
}
