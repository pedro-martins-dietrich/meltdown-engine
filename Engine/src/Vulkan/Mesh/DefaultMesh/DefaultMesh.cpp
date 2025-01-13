#include <pch.hpp>
#include "DefaultMesh.hpp"

#include "../ObjMeshLoader.hpp"

mtd::DefaultMesh::DefaultMesh
(
	const Device& device,
	uint32_t index,
	const char* id,
	const char* fileName,
	const MaterialInfo& materialInfo,
	const std::vector<Mat4x4>& preTransforms
) : Mesh{device, index, id, preTransforms, 1}, material{materialInfo}, indexOffset{0}
{
	ObjMeshLoader::loadDefault3DMesh(fileName, vertices, indices, material);
}

mtd::DefaultMesh::DefaultMesh(DefaultMesh&& other) noexcept
	: Mesh{std::move(other)},
	vertices{std::move(other.vertices)},
	indices{std::move(other.indices)},
	material{std::move(other.material)},
	indexOffset{other.indexOffset}
{
}

// Loads mesh texture
void mtd::DefaultMesh::loadTexture
(
	const Device& device, const CommandHandler& commandHandler, DescriptorSetHandler& descriptorSetHandler
)
{
	material.loadMaterial(device, commandHandler, descriptorSetHandler, meshIndex);
}
