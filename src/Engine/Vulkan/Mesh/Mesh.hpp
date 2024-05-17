#pragma once

#include "../../Utils/EngineStructs.hpp"

namespace mtd
{
	// Stores a mesh
	class Mesh
	{
		public:
			Mesh(const char* fileName, uint32_t id, glm::mat4 preTransform = glm::mat4{1.0f});
			~Mesh() {}

			Mesh(const Mesh&) = delete;
			Mesh& operator=(const Mesh&) = delete;

			Mesh(Mesh&& other) noexcept;

			// Getters
			uint32_t getMeshID() const { return id; }
			const std::vector<Vertex>& getVertices() const { return vertices; }
			const std::vector<uint32_t>& getIndices() const { return indices; }
			const std::string& getTexturePath() const { return diffuseTexturePath; }
			uint32_t getInstanceCount() const { return static_cast<uint32_t>(transforms.size()); }
			const std::vector<glm::mat4>& getTransformationMatrices() const
				{ return transforms; }
			glm::mat4 getTransformationMatrix(uint32_t instance) const
				{ return transforms[instance]; }
			vk::DeviceSize getModelMatricesSize() const
				{ return sizeof(glm::mat4) * transforms.size(); }

			// Sets the write location for the transformation matrices
			void setTransformsWriteLocation(void* location)
				{ transformsMemoryLocation = static_cast<glm::mat4*>(location); }

			// Adds a new instance of the mesh
			void addInstance(glm::mat4 preTransform = glm::mat4{1.0f});

			// Writes the transformation matrices in the GPU mapped memory
			void updateTransformationMatricesDescriptor() const;
			void updateTransformationMatrix(glm::mat4 newTransform, uint32_t instance);

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

			// Transformation matrices for each instance
			std::vector<glm::mat4> transforms;
			// Memory location for the transformation matrices descriptor
			glm::mat4* transformsMemoryLocation;

			// Mesh data
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			// Mesh texture file path
			std::string diffuseTexturePath;
	};
}
