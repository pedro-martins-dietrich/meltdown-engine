#include "Mesh.hpp"

mtd::Mesh::Mesh(const glm::mat4& preTransform)
	: transforms{preTransform}, instanceLumpOffset{0}, pInstanceLump{nullptr}
{
}

mtd::Mesh::Mesh
(
	std::vector<glm::mat4>&& transforms,
	size_t instanceLumpOffset,
	std::vector<glm::mat4>* pInstanceLump
) : transforms{transforms}, instanceLumpOffset{instanceLumpOffset}, pInstanceLump{pInstanceLump}
{
}

// Sets a reference to the instance lump to update the instances data
void mtd::Mesh::setInstancesLump(std::vector<glm::mat4>* instanceLumpPointer, size_t offset)
{
	pInstanceLump = instanceLumpPointer;
	instanceLumpOffset = offset;
}

// Adds a new instance
void mtd::Mesh::addInstance(glm::mat4 preTransform)
{
	transforms.push_back(preTransform);
}

// Writes the transformation matrices in the GPU mapped memory
void mtd::Mesh::updateTransformationMatrix(glm::mat4 newTransform, uint32_t instance)
{
	transforms[instance] = newTransform;
	(*pInstanceLump)[instanceLumpOffset + instance] = newTransform;
}
