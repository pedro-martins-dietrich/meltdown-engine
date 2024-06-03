#include "BillboardManager.hpp"

#include "../../../Utils/Logger.hpp"

mtd::BillboardManager::BillboardManager(const Device& device) : device{device}
{
}

mtd::BillboardManager::~BillboardManager()
{
}

// Loads the textures of the billboards
void mtd::BillboardManager::loadTextures
(
	const CommandHandler& commandHandler, DescriptorSetHandler& textureDescriptorSetHandler
)
{
	textures.resize(getMeshCount());
	for(uint32_t i = 0; i < getMeshCount(); i++)
	{
		textures[i] = std::make_unique<Texture>
		(
			device,
			"textures/orb.png",
			commandHandler,
			textureDescriptorSetHandler,
			i
		);
	}

	LOG_VERBOSE("Billboard textures loaded.");
}

// Draws the mesh specified by the index
void mtd::BillboardManager::drawMesh(const vk::CommandBuffer& commandBuffer, uint32_t index) const
{
	commandBuffer.draw(6, 1, 0, index);
}
