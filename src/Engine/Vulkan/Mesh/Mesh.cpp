#include "Mesh.hpp"

#include "ObjMeshLoader.hpp"

mtd::Mesh::Mesh(const char* fileName, uint32_t id, glm::mat4 preTransform)
	: id{id}, transforms{preTransform}, transformsMemoryLocation{nullptr}
{
	ObjMeshLoader::load(fileName, vertices, indices);
}

mtd::Mesh::Mesh(Mesh&& other) noexcept
	: id{other.id},
	transforms{std::move(other.transforms)},
	vertices{std::move(other.vertices)},
	indices{std::move(other.indices)},
	transformsMemoryLocation{other.transformsMemoryLocation}
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
