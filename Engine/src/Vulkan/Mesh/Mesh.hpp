#pragma once

#include <string>
#include <vector>

#include <meltdown/model.hpp>

namespace mtd
{
	// Generic mesh data
	class Mesh
	{
		public:
			Mesh(uint32_t index, const char* modelID, const Mat4x4& preTransform);
			Mesh
			(
				uint32_t index,
				const std::string& modelID,
				std::vector<std::unique_ptr<Model>>&& models,
				size_t instanceLumpOffset,
				std::vector<Mat4x4>* pInstanceLump
			);
			virtual ~Mesh() = default;

			Mesh(const Mesh&) = delete;
			Mesh& operator=(const Mesh&) = delete;

			// Getters
			uint32_t getInstanceCount() const { return static_cast<uint32_t>(models.size()); }
			uint32_t getInstanceOffset() const { return static_cast<uint32_t>(instanceLumpOffset); }

			// Runs once at the beginning of the scene for all instances
			void start();
			// Updates all instances
			void update(double deltaTime);

			// Sets a reference to the instance lump to update the instances data
			void setInstancesLump(std::vector<Mat4x4>* instanceLumpPointer, size_t offset);
			// Adds a new instance
			void addInstance(const Mat4x4& preTransform = Mat4x4{1.0f});

		protected:
			// Mesh index
			uint32_t meshIndex;
			// Model ID
			std::string modelID;
			// Model instance factory
			ModelFactory modelFactory;

			// Model data for each instance of the mesh
			std::vector<std::unique_ptr<Model>> models;

			// Pointer to the instance lump vector and start index
			size_t instanceLumpOffset;
			std::vector<Mat4x4>* pInstanceLump;
	};
}
