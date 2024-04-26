#include "Mesh.hpp"

mtd::Mesh::Mesh(uint32_t id, glm::mat4 preTransform) : id{id}, preTransforms{preTransform}, instanceCount{1}
{
	// Hard-coded meshes until creating a mesh loader
	if(id == 0)
	{
		// Triangle
		vertices =
		{
			Vertex
			{
				glm::vec3{1.0f, 1.0f, 0.0f},
				glm::vec3{0.0f, 0.0f, 1.0f},
				glm::vec3{1.0f, 0.0f, 0.0f}
			},
			Vertex
			{
				glm::vec3{1.0f, -1.0f, -1.0f},
				glm::vec3{0.0f, -1.0f, 0.0f},
				glm::vec3{0.0f, 1.0f, 0.0f}
			},
			Vertex
			{
				glm::vec3{1.0f, 1.0f, -2.0f},
				glm::vec3{0.0f, 0.0f, -1.0f},
				glm::vec3{0.0f, 0.0f, 1.0f}
			}
		};
		indices = {0, 1, 2};
		return;
	}

	// Square
	vertices =
	{
		Vertex
		{
			glm::vec3{-1.0f, 1.0f, 0.0f},
			glm::vec3{0.0f, 0.0f, 1.0f},
			glm::vec3{0.0f, 0.0f, 1.0f}
		},
		Vertex
		{
			glm::vec3{-1.0f, -1.0f, 0.0f},
			glm::vec3{0.0f, 0.0f, 1.0f},
			glm::vec3{0.0f, 1.0f, 1.0f}
		},
		Vertex
		{
			glm::vec3{1.0f, -1.0f, 0.0f},
			glm::vec3{0.0f, 0.0f, 1.0f},
			glm::vec3{1.0f, 1.0f, 1.0f}
		},
		Vertex
		{
			glm::vec3{1.0f, 1.0f, 0.0f},
			glm::vec3{0.0f, 0.0f, 1.0f},
			glm::vec3{1.0f, 0.0f, 1.0f}
		}
	};
	indices = {0, 1, 2, 0, 3, 2};
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
