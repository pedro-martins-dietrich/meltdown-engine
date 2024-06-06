#include "BillboardManager.hpp"

#include "../../../Utils/Logger.hpp"

mtd::BillboardManager::BillboardManager(const Device& device) : device{device}
{
	instanceLump.push_back(glm::mat4
	{
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, -2.0f, 0.0f, 1.0f
	});

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
}

mtd::BillboardManager::~BillboardManager()
{
	const vk::Device& vulkanDevice = device.getDevice();

	vulkanDevice.destroyBuffer(instanceBuffer.buffer);
	vulkanDevice.freeMemory(instanceBuffer.bufferMemory);
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

// Updates instances data
void mtd::BillboardManager::update() const
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
	commandBuffer.draw(6, 1, 0, index);
}
