#include "DefaultMesh.hpp"

#include "../ObjMeshLoader.hpp"

mtd::DefaultMesh::DefaultMesh(uint32_t id, const char* fileName, glm::mat4 preTransform)
	: Mesh{preTransform}, id{id}
{
	ObjMeshLoader::load(fileName, vertices, indices, diffuseTexturePath);
}

mtd::DefaultMesh::DefaultMesh(DefaultMesh&& other) noexcept
	: Mesh{std::move(other.transforms), other.instanceLumpOffset, other.pInstanceLump},
	id{other.id},
	vertices{std::move(other.vertices)},
	indices{std::move(other.indices)},
	diffuseTexturePath{std::move(other.diffuseTexturePath)},
	diffuseTexture{std::move(other.diffuseTexture)}
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
		device, diffuseTexturePath.c_str(), commandHandler, descriptorSetHandler, id
	);
}
