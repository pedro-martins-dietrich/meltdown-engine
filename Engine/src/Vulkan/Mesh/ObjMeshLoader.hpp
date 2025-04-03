#pragma once

#include "../../Material/Material.hpp"
#include "../../Material/MaterialLump.hpp"

// Responsible for loading Wavefront .obj files
namespace mtd::ObjMeshLoader
{
	// Loads a default 3D mesh from file
	void loadDefault3DMesh
	(
		const char* fileName,
		std::vector<Vertex>& vertices,
		std::vector<uint32_t>& indices,
		Material& meshMaterial
	);

	// Loads a 3D mesh with multiple materials from file
	void loadMultiMaterial3DMesh
	(
		const char* fileName,
		std::vector<Vertex>& vertices,
		std::vector<uint32_t>& indices,
		std::vector<SubmeshData>& submeshInfos,
		std::vector<Material>& meshMaterials,
		const MaterialInfo& materialInfo
	);

	// Loads a 3D mesh for ray tracing from file
	void loadRayTracingMesh
	(
		const char* fileName,
		std::vector<Vertex>& vertices,
		std::vector<uint32_t>& indices,
		std::vector<uint16_t>& materialIndices,
		MaterialLump& materialLump
	);
}
