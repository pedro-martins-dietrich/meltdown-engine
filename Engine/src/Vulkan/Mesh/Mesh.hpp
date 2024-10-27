#pragma once

#include <string>
#include <vector>

#include <meltdown/model.hpp>

#include "../Device/Memory.hpp"

namespace mtd
{
	// Base class for handling meshes in Vulkan
	class Mesh
	{
		public:
			Mesh
			(
				const Device& device,
				uint32_t index,
				const char* modelID,
				const Mat4x4& preTransform,
				uint32_t instanceBufferBindIndex = 0
			);
			virtual ~Mesh();

			Mesh(const Mesh&) = delete;
			Mesh& operator=(const Mesh&) = delete;

			Mesh(Mesh&& other) noexcept;

			// Getters
			uint32_t getInstanceCount() const { return static_cast<uint32_t>(models.size()); }

			// Runs once at the beginning of the scene for all instances
			void start();
			// Updates all instances
			void update(double deltaTime);

			// Adds a new instance
			void addInstance(const Mat4x4& preTransform = Mat4x4{1.0f});

			// Creates a GPU buffer for the transformation matrices
			void createInstanceBuffer();

			// Binds the instance buffer for this mesh
			void bindInstanceBuffer(const vk::CommandBuffer& commandBuffer) const;

		protected:
			// Mesh index
			uint32_t meshIndex;
			// Model ID
			std::string modelID;
			// Model instance factory
			ModelFactory modelFactory;

			// Model data for each instance of the mesh
			std::vector<std::unique_ptr<Model>> models;

			// Transformation matrices for each mesh instance
			std::vector<Mat4x4> instanceLump;
			// GPU buffer for the transformation matrices
			Memory::Buffer instanceBuffer;
			// Binding index for the instance buffer
			uint32_t instanceBufferBindIndex;

			// Device reference
			const Device& device;
	};
}
