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

			// Sets a reference to the instance lump to update the instances data
			void setInstancesLump(std::vector<glm::mat4>* instanceLumpPointer, size_t offset);

			// Adds a new instance of the mesh
			void addInstance(glm::mat4 preTransform = glm::mat4{1.0f});

			// Writes the transformation matrices in the GPU mapped memory
			void updateTransformationMatrix(glm::mat4 newTransform, uint32_t instance);

		private:
			// Mesh ID
			uint32_t id;

			// Mesh data
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			// Transformation matrices for each instance
			std::vector<glm::mat4> transforms;
			
			// Mesh texture file path
			std::string diffuseTexturePath;

			// Pointer to the instance lump vector and start index
			size_t instanceLumpOffset;
			std::vector<glm::mat4>* pInstanceLump;
	};
}
