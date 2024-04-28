#include "Mesh.hpp"

#include "ObjMeshLoader.hpp"

mtd::Mesh::Mesh(const char* fileName, uint32_t id, glm::mat4 preTransform)
	: id{id}, preTransforms{preTransform}, instanceCount{1}
{
	ObjMeshLoader::load(fileName, vertices, indices);
}

// Adds a new instance of the mesh
void mtd::Mesh::addInstance(glm::mat4 preTransform)
{
	preTransforms.push_back(preTransform);
	instanceCount++;
}

// Floats per vertex
const uint32_t mtd::Mesh::floatsPerVertex = sizeof(Vertex) / sizeof(float);

// Mesh binding description
const vk::VertexInputBindingDescription mtd::Mesh::bindingDescription
{
	0,										// Binding
	static_cast<uint32_t>(sizeof(Vertex)),	// Stride
	vk::VertexInputRate::eVertex			// Input rate
};

// Mesh attribute descriptions
const std::vector<vk::VertexInputAttributeDescription> mtd::Mesh::attributeDescriptions
{
	// Position
	vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, 0},
	// Normal vectors
	vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat, 3 * sizeof(float)},
	// Color
	vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32B32Sfloat, 6 * sizeof(float)}
};
