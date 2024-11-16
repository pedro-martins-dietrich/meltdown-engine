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
				const std::vector<Mat4x4>& preTransforms,
				uint32_t instanceBufferBindIndex = 0
			);
			virtual ~Mesh();

			Mesh(const Mesh&) = delete;
			Mesh& operator=(const Mesh&) = delete;

			Mesh(Mesh&& other) noexcept;

			// Getters
			uint32_t getInstanceCount() const { return static_cast<uint32_t>(models.size()); }
			const char* getModelID() const { return modelID.c_str(); }

			// Runs once at the beginning of the scene for all instances
			void start();
			// Updates all instances
			void update(double deltaTime);

			// Adds multiple new mesh instances with the identity pre-transform matrix
			void addInstances(const CommandHandler& commandHandler, uint32_t instanceCount);
			// Removes the mesh instance associated with the provided instance ID
			void removeInstanceByID(const CommandHandler& commandHandler, uint64_t instanceID);

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
			std::unordered_map<uint64_t, std::unique_ptr<Model>> models;

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
