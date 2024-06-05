#pragma once

#include <memory>

#include "MeshManager.hpp"
#include "../../Image/Texture.hpp"

namespace mtd
{
	class BillboardManager : public MeshManager
	{
		public:
			BillboardManager(const Device& device);
			~BillboardManager();

			BillboardManager(const BillboardManager&) = delete;
			BillboardManager& operator=(const BillboardManager&) = delete;

			// Getter
			virtual uint32_t getMeshCount() const override { return 1; }

			// Loads the textures of the billboards
			virtual void loadTextures
			(
				const CommandHandler& commandHandler,
				DescriptorSetHandler& textureDescriptorSetHandler
			) override;

			// Updates instances data
			virtual void update() const override;

			// Binds the vertex buffer for instances data
			virtual void bindBuffers(const vk::CommandBuffer& commandBuffer) const override;
			// Draws the mesh specified by the index
			virtual void drawMesh
			(
				const vk::CommandBuffer& commandBuffer, uint32_t index
			) const override;

		private:
			// Transformation matrices for each instance
			Memory::Buffer instanceBuffer;

			// Textures for each billboard
			std::vector<std::unique_ptr<Texture>> textures;

			// Instances transforms lump
			std::vector<glm::mat4> instanceLump;

			// Device reference
			const Device& device;
	};
}