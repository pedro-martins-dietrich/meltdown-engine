#include "Mesh.hpp"

#include "ObjMeshLoader.hpp"

mtd::Mesh::Mesh(const char* fileName, uint32_t id, glm::mat4 preTransform)
	: id{id},
	transforms{preTransform},
	instanceLumpOffset{0},
	pInstanceLump{nullptr}
{
	ObjMeshLoader::load(fileName, vertices, indices, diffuseTexturePath);
}

mtd::Mesh::Mesh(Mesh&& other) noexcept
	: id{other.id},
	vertices{std::move(other.vertices)},
	indices{std::move(other.indices)},
	transforms{std::move(other.transforms)},
	diffuseTexturePath{std::move(other.diffuseTexturePath)},
	instanceLumpOffset{other.instanceLumpOffset},
	pInstanceLump{other.pInstanceLump}
{
	pInstanceLump = nullptr;
}

void mtd::Mesh::setInstancesLump(std::vector<glm::mat4>* instanceLumpPointer, size_t offset)
{
	pInstanceLump = instanceLumpPointer;
	instanceLumpOffset = offset;
}

// Adds a new instance of the mesh
void mtd::Mesh::addInstance(glm::mat4 preTransform)
{
	transforms.push_back(preTransform);
}

void mtd::Mesh::updateTransformationMatrix(glm::mat4 newTransform, uint32_t instance)
{
	transforms[instance] = newTransform;
	(*pInstanceLump)[instanceLumpOffset + instance] = newTransform;
}
