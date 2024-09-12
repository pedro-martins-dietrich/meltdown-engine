#include "BillboardManager.hpp"

#include "../../../Utils/Logger.hpp"

mtd::BillboardManager::BillboardManager(const Device& device) : device{device}
{
}

mtd::BillboardManager::~BillboardManager()
{
	clearMeshes();
}

// Loads the billboards textures to the GPU
void mtd::BillboardManager::loadMeshes
(
	const CommandHandler& commandHandler, DescriptorSetHandler& textureDescriptorSetHandler
)
{
	for(Billboard& billboard: billboards)
	{
		billboard.setInstancesLump(&instanceLump, instanceLump.size());

		billboard.loadTexture(device, commandHandler, textureDescriptorSetHandler);
	}

	vk::DeviceSize instanceLumpSize = instanceLump.size() * sizeof(Mat4x4);
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

// Clears the list of billboards and the instance buffer
void mtd::BillboardManager::clearMeshes()
{
	if(getMeshCount() == 0)
		return;

	const vk::Device& vulkanDevice = device.getDevice();
	vulkanDevice.waitIdle();

	instanceLump.clear();
	billboards.clear();

	vulkanDevice.destroyBuffer(instanceBuffer.buffer);
	vulkanDevice.freeMemory(instanceBuffer.bufferMemory);
}

// Executes the start code for each model on scene loading
void mtd::BillboardManager::start()
{
	for(Billboard& billboard: billboards)
		billboard.start();

	Memory::copyMemory
	(
		device.getDevice(),
		instanceBuffer.bufferMemory,
		instanceLump.size() * sizeof(Mat4x4),
		instanceLump.data()
	);
}

// Updates instances data
void mtd::BillboardManager::update(double frameTime)
{
	for(Billboard& billboard: billboards)
		billboard.update(frameTime);

	Memory::copyMemory
	(
		device.getDevice(),
		instanceBuffer.bufferMemory,
		instanceLump.size() * sizeof(Mat4x4),
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
