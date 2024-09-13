#include <pch.hpp>
#include "DefaultMesh.hpp"

#include "../ObjMeshLoader.hpp"

mtd::DefaultMesh::DefaultMesh
(
	uint32_t index, const char* id, const char* fileName, const Mat4x4& preTransform
) : Mesh{index, id, preTransform}, indexOffset{0}
{
	ObjMeshLoader::load(fileName, vertices, indices, diffuseTexturePath);
}

mtd::DefaultMesh::DefaultMesh(DefaultMesh&& other) noexcept
	: Mesh
	{
		other.meshIndex,
		other.modelID,
		std::move(other.models),
		other.instanceLumpOffset,
		other.pInstanceLump
	},
	vertices{std::move(other.vertices)},
	indices{std::move(other.indices)},
	diffuseTexturePath{std::move(other.diffuseTexturePath)},
	diffuseTexture{std::move(other.diffuseTexture)},
	indexOffset{other.indexOffset}
{
	other.diffuseTexture = nullptr;
	other.pInstanceLump = nullptr;
}

// Loads mesh texture
void mtd::DefaultMesh::loadTexture
(
	const Device& device,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler
)
{
	diffuseTexture = std::make_unique<Texture>
	(
		device, diffuseTexturePath.c_str(), commandHandler, descriptorSetHandler, meshIndex
	);
}
