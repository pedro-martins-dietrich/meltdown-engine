#pragma once

#include "../Descriptors/DescriptorSetHandler.hpp"

namespace mtd
{
	// Interface for different mesh managers
	class MeshManager
	{
		public:
			virtual ~MeshManager() = default;

			// Gets the number of different meshes handled by the manager
			virtual uint32_t getMeshCount() const = 0;

			// Loads mesh data to the GPU
			virtual void loadMeshes
			(
				const CommandHandler& commandHandler,
				DescriptorSetHandler& textureDescriptorSetHandler
			) = 0;

			// Clears mesh data
			virtual void clearMeshes() = 0;

			// Updates mesh data
			virtual void update(double frameTime) = 0;

			// Binds vertex and index buffers, if used
			virtual void bindBuffers(const vk::CommandBuffer& commandBuffer) const = 0;
			// Draws the mesh specified by the index
			virtual void drawMesh
			(
				const vk::CommandBuffer& commandBuffer, uint32_t index
			) const = 0;
	};
}
