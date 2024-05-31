#pragma once

#include <vulkan/vulkan.hpp>

namespace mtd
{
	// Interface for different mesh managers
	class MeshManager
	{
		public:
			virtual ~MeshManager() = default;

			// Gets the number of different meshes handled by the manager
			virtual uint32_t getMeshCount() const = 0;

			// Binds vertex and index buffers, if used
			virtual void bindBuffers(const vk::CommandBuffer& commandBuffer) const = 0;
			// Draws the mesh specified by the index
			virtual void drawMesh
			(
				const vk::CommandBuffer& commandBuffer, uint32_t index
			) const = 0;

		protected:
	};
}
