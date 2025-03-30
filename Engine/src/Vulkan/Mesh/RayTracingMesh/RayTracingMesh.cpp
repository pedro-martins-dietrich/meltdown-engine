#include <pch.hpp>
#include "RayTracingMesh.hpp"

#include "../ObjMeshLoader.hpp"

mtd::RayTracingMesh::RayTracingMesh
(
	const Device& device,
	uint32_t index,
	const char* id,
	const char* fileName,
	const MaterialInfo& materialInfo,
	const std::vector<Mat4x4>& preTransforms
) : Mesh{device, index, id, preTransforms, 1}
{
	ObjMeshLoader::loadRayTracingMesh(fileName, vertices, indices, materialIndices, materials, materialInfo);
}

mtd::RayTracingMesh::RayTracingMesh(RayTracingMesh&& other) noexcept
	: Mesh{std::move(other)},
	vertices{std::move(other.vertices)},
	indices{std::move(other.indices)},
	materialIndices{std::move(other.materialIndices)},
	materials{std::move(other.materials)}
{}

uint32_t mtd::RayTracingMesh::getTextureCount() const
{
	if(materials.empty()) return 0;
	return static_cast<uint32_t>(materials[0].getTextureCount() * materials.size());
}

// Checks if the used material has float data attributes
bool mtd::RayTracingMesh::hasMaterialFloatData() const
{
	if(materials.empty()) return false;
	return materials[0].hasFloatData();
}

// Loads mesh materials
void mtd::RayTracingMesh::loadMaterials
(
	const Device& device,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t initialMaterialIndex
)
{
	for(uint32_t i = 0; i < materials.size(); i++)
		materials[i].loadMaterial(device, commandHandler, descriptorSetHandler, 0, 4);
}

// Deletes all mesh data on this object
void mtd::RayTracingMesh::clearMeshData()
{
	vertices.clear();
	indices.clear();
	materialIndices.clear();
}
