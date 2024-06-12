#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace mtd
{
	// Generic mesh data
	class Mesh
	{
		public:
			Mesh(const glm::mat4& preTransform);
			Mesh
			(
				std::vector<glm::mat4>&& transforms,
				size_t instanceLumpOffset,
				std::vector<glm::mat4>* pInstanceLump
			);
			virtual ~Mesh() = default;

			Mesh(const Mesh&) = delete;
			Mesh& operator=(const Mesh&) = delete;

			// Getters
			uint32_t getInstanceCount() const { return static_cast<uint32_t>(transforms.size()); }
			uint32_t getInstanceOffset() const { return static_cast<uint32_t>(instanceLumpOffset); }
			const std::vector<glm::mat4>& getTransformationMatrices() const
				{ return transforms; }
			glm::mat4 getTransformationMatrix(uint32_t instance) const
				{ return transforms[instance]; }

			// Sets a reference to the instance lump to update the instances data
			void setInstancesLump(std::vector<glm::mat4>* instanceLumpPointer, size_t offset);
			// Adds a new instance
			void addInstance(glm::mat4 preTransform = glm::mat4{1.0f});
			// Writes the transformation matrices in the GPU mapped memory
			void updateTransformationMatrix(glm::mat4 newTransform, uint32_t instance);

		protected:
			// Transformation matrices for each instance
			std::vector<glm::mat4> transforms;

			// Pointer to the instance lump vector and start index
			size_t instanceLumpOffset;
			std::vector<glm::mat4>* pInstanceLump;
	};
}
