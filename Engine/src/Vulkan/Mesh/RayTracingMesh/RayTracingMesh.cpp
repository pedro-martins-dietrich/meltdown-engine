#include <pch.hpp>
#include "RayTracingMesh.hpp"

#include "../ObjMeshLoader.hpp"

mtd::RayTracingMesh::RayTracingMesh
(
	const Device& device,
	uint32_t index,
	const char* id,
	const char* fileName,
	MaterialLump& materialLump,
	const std::vector<Mat4x4>& preTransforms
) : Mesh{device, index, id, preTransforms, 1}, materialCount{0U}
{
	uint32_t materialOffset = materialLump.getMaterialCount();
	ObjMeshLoader::loadRayTracingMesh(fileName, vertices, indices, materialIndices, materialLump);
	materialCount = materialLump.getMaterialCount() - materialOffset;
}

mtd::RayTracingMesh::RayTracingMesh(RayTracingMesh&& other) noexcept
	: Mesh{std::move(other)},
	vertices{std::move(other.vertices)},
	indices{std::move(other.indices)},
	materialIndices{std::move(other.materialIndices)},
	materialCount{other.materialCount}
{}

// Deletes all mesh data on this object
void mtd::RayTracingMesh::clearMeshData()
{
	vertices.clear();
	indices.clear();
	materialIndices.clear();
}
