#include <pch.hpp>
#include "DefaultMesh.hpp"

#include "../ObjMeshLoader.hpp"

mtd::DefaultMesh::DefaultMesh
(
	const Device& device,
	uint32_t index,
	const char* id,
	const char* fileName,
	const std::vector<Mat4x4>& preTransforms
) : Mesh{device, index, id, preTransforms, 1}, indexOffset{0}
{
	ObjMeshLoader::loadDefault3DMesh(fileName, vertices, indices, diffuseTexturePath);
}

mtd::DefaultMesh::DefaultMesh(DefaultMesh&& other) noexcept
	: Mesh{std::move(other)},
	vertices{std::move(other.vertices)},
	indices{std::move(other.indices)},
	diffuseTexturePath{std::move(other.diffuseTexturePath)},
	diffuseTexture{std::move(other.diffuseTexture)},
	indexOffset{other.indexOffset}
{
	other.diffuseTexture = nullptr;
}

// Loads mesh texture
void mtd::DefaultMesh::loadTexture
(
	const Device& device,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler
)
{
	diffuseTexture =
		std::make_unique<Texture>(device, diffuseTexturePath.c_str(), commandHandler, descriptorSetHandler, meshIndex);
}
