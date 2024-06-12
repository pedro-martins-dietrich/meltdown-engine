#include "BillboardManager.hpp"

#include "../../../Utils/Logger.hpp"

mtd::BillboardManager::BillboardManager(const Device& device) : device{device}
{
}

mtd::BillboardManager::~BillboardManager()
{
	const vk::Device& vulkanDevice = device.getDevice();

	vulkanDevice.destroyBuffer(instanceBuffer.buffer);
	vulkanDevice.freeMemory(instanceBuffer.bufferMemory);
}

// Loads the billboards textures to the GPU
void mtd::BillboardManager::loadMeshes
(
	const CommandHandler& commandHandler, DescriptorSetHandler& textureDescriptorSetHandler
)
{
	for(Billboard& billboard: billboards)
	{
		const std::vector<glm::mat4>& instancesData = billboard.getTransformationMatrices();
		billboard.setInstancesLump(&instanceLump, instanceLump.size());
		instanceLump.insert(instanceLump.end(), instancesData.begin(), instancesData.end());

		billboard.loadTexture(device, commandHandler, textureDescriptorSetHandler);
	}

	vk::DeviceSize instanceLumpSize = instanceLump.size() * sizeof(glm::mat4);
	Memory::createBuffer
	(
		device,
		instanceBuffer,
		instanceLumpSize,
		vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	Memory::copyMemory
	(
		device.getDevice(), instanceBuffer.bufferMemory, instanceLumpSize, instanceLump.data()
	);

	LOG_VERBOSE("Billboards loaded.");
}

// Updates instances data
void mtd::BillboardManager::update(double frameTime)
{
	Memory::copyMemory
	(
		device.getDevice(),
		instanceBuffer.bufferMemory,
		instanceLump.size() * sizeof(glm::mat4),
		instanceLump.data()
	);
}

// Binds the vertex buffer for instances data
void mtd::BillboardManager::bindBuffers(const vk::CommandBuffer& commandBuffer) const
{
	vk::DeviceSize offset = 0;
	commandBuffer.bindVertexBuffers(0, 1, &(instanceBuffer.buffer), &offset);
}

// Draws the mesh specified by the index
void mtd::BillboardManager::drawMesh(const vk::CommandBuffer& commandBuffer, uint32_t index) const
{
	const Billboard& billboard = billboards[index];
	commandBuffer.drawIndexed(6, billboard.getInstanceCount(), 0, 0, billboard.getInstanceOffset());
}
