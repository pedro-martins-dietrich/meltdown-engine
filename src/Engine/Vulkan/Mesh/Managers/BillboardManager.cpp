#include "BillboardManager.hpp"

mtd::BillboardManager::BillboardManager(const Device& device) : device{device}
{
}

mtd::BillboardManager::~BillboardManager()
{
}

// Draws the mesh specified by the index
void mtd::BillboardManager::drawMesh(const vk::CommandBuffer& commandBuffer, uint32_t index) const
{
	commandBuffer.draw(6, 1, 0, index);
}
