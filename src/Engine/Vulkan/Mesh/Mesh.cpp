#include "Mesh.hpp"

#include "ObjMeshLoader.hpp"

mtd::Mesh::Mesh(const char* fileName, uint32_t id, glm::mat4 preTransform)
	: id{id}, transforms{preTransform}, transformsMemoryLocation{nullptr}
{
	ObjMeshLoader::load(fileName, vertices, indices, diffuseTexturePath);
}

mtd::Mesh::Mesh(Mesh&& other) noexcept
	: id{other.id},
	transforms{std::move(other.transforms)},
	vertices{std::move(other.vertices)},
	indices{std::move(other.indices)},
	transformsMemoryLocation{other.transformsMemoryLocation},
	diffuseTexturePath{std::move(other.diffuseTexturePath)}
{
	transformsMemoryLocation = nullptr;
}

// Adds a new instance of the mesh
void mtd::Mesh::addInstance(glm::mat4 preTransform)
{
	transforms.push_back(preTransform);
}

// Writes the transformation matrices in the GPU mapped memory
void mtd::Mesh::updateTransformationMatricesDescriptor() const
{
	memcpy(transformsMemoryLocation, transforms.data(), getModelMatricesSize());
}

void mtd::Mesh::updateTransformationMatrix(glm::mat4 newTransform, uint32_t instance)
{
	transforms[instance] = newTransform;

	memcpy(transformsMemoryLocation + instance, &transforms[instance], sizeof(glm::mat4));
}
