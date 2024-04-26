#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace mtd
{
	// Vertex format
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
	};

	// Stores a mesh
	class Mesh
	{
		public:
			Mesh(uint32_t id, glm::mat4 preTransform = glm::mat4{1.0f});
			~Mesh() {}

			Mesh(const Mesh&) = delete;
			Mesh& operator=(const Mesh&) = delete;

			// Getters
			uint32_t getMeshID() const { return id; }
			const std::vector<Vertex>& getVertices() const { return vertices; }
			const std::vector<uint32_t>& getIndices() const { return indices; }
			uint32_t getInstanceCount() const { return instanceCount; }

			// Adds a new instance of the mesh
			void addInstance(glm::mat4 preTransform = glm::mat4{1.0f});

			// Gets mesh data
			static const vk::VertexInputBindingDescription&
				getInputBindingDescription() { return bindingDescription; }
			static const std::vector<vk::VertexInputAttributeDescription>&
				getInputAttributeDescriptions() { return attributeDescriptions; }

		private:
			// Floats per vertex
			static const uint32_t floatsPerVertex;
			// Vertex input data
			static const vk::VertexInputBindingDescription bindingDescription;
			static const std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

			// Mesh ID
			uint32_t id;
			// Instance count
			uint32_t instanceCount;
			// Pre-transformations for each instance
			std::vector<glm::mat4> preTransforms;

			// Vertex data
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
	};
}
