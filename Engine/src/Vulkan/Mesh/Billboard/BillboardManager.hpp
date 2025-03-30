#pragma once

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
			~BillboardManager() = default;

			// Getters
			virtual uint32_t getMaterialCount() const override { return static_cast<uint32_t>(meshes.size()); }
			virtual uint32_t getTextureCount() const override;

			// Checks if the material type for the stored meshes has float data
			virtual bool hasMaterialFloatData() const override;

			// Loads the billboards textures to the GPU
			virtual void loadMeshes(DescriptorSetHandler& textureDescriptorSetHandler) override;

			// There is no buffer common to all billboards to be binded
			virtual void bindBuffers(const vk::CommandBuffer& commandBuffer) const override;
			// Draws the meshes using a rasterization pipeline
			virtual void drawMesh
			(
				const vk::CommandBuffer& commandBuffer, const GraphicsPipeline& graphicsPipeline
			) const override;
	};
}
