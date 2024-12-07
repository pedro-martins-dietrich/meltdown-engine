#pragma once

#include <memory>

#include "Billboard.hpp"
#include "../BaseMeshManager.hpp"
#include "../../Image/Texture.hpp"

namespace mtd
{
	// Handles data from all billboards
	class BillboardManager : public BaseMeshManager<Billboard>
	{
		public:
			BillboardManager(const Device& device);
			~BillboardManager();

			// Gets the total number of textures handled by the manager
			virtual uint32_t getTextureCount() const override { return static_cast<uint32_t>(meshes.size()); }

			// Loads the billboards textures to the GPU
			virtual void loadMeshes(DescriptorSetHandler& textureDescriptorSetHandler) override;

			// Clears the list of billboards and the instance buffer
			virtual void clearMeshes() override;

			// There is no buffer common to all billboards to be binded
			virtual void bindBuffers(const vk::CommandBuffer& commandBuffer) const override;
			// Draws the mesh specified by the index
			virtual void drawMesh(const vk::CommandBuffer& commandBuffer, const Pipeline& pipeline) const override;
	};
}
