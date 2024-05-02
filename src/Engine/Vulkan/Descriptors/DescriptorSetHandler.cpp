#include "DescriptorSetHandler.hpp"

#include "../../Utils/Logger.hpp"

mtd::DescriptorSetHandler::DescriptorSetHandler
(
	const vk::Device& device, const std::vector<vk::DescriptorSetLayoutBinding>& setLayoutBindings
) : device{device}
{
	createDescriptorSetLayout(setLayoutBindings);
}

mtd::DescriptorSetHandler::~DescriptorSetHandler()
{
	for(DescriptorResources& resources: resourcesList)
	{
		if(resources.descriptorBuffer.bufferMemory)
		{
			device.unmapMemory(resources.descriptorBuffer.bufferMemory);
			device.freeMemory(resources.descriptorBuffer.bufferMemory);
			device.destroyBuffer(resources.descriptorBuffer.buffer);
		}
	}

	device.destroyDescriptorSetLayout(descriptorSetLayout);
}

mtd::DescriptorSetHandler::DescriptorSetHandler(DescriptorSetHandler&& other) noexcept
	: device{other.device},
	descriptorSetLayout{std::move(other.descriptorSetLayout)},
	descriptorSet{std::move(other.descriptorSet)},
	resourcesList{std::move(other.resourcesList)}
{
	other.descriptorSetLayout = nullptr;
	other.descriptorSet = nullptr;
}

// Creates a descriptor and assings it to a descriptor set
void mtd::DescriptorSetHandler::createDescriptorResources
(
	const Device& mtdDevice,
	vk::DeviceSize resourceSize,
	vk::BufferUsageFlags usageFlags,
	uint32_t resourceIndex
)
{
	Memory::createBuffer
	(
		mtdDevice,
		resourcesList[resourceIndex].descriptorBuffer,
		resourceSize,
		usageFlags,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	resourcesList[resourceIndex].descriptorBufferWriteLocation = device.mapMemory
	(
		resourcesList[resourceIndex].descriptorBuffer.bufferMemory, 0UL, resourceSize
	);
	resourcesList[resourceIndex].descriptorBufferInfo.buffer =
		resourcesList[resourceIndex].descriptorBuffer.buffer;
	resourcesList[resourceIndex].descriptorBufferInfo.offset = 0UL;
	resourcesList[resourceIndex].descriptorBufferInfo.range = resourceSize;
}

// Updates the descriptor set data
void mtd::DescriptorSetHandler::writeDescriptorSet()
{
	writeOps.resize(resourcesList.size());
	for(uint32_t i = 0; i < resourcesList.size(); i++)
	{
		writeOps[i].dstSet = descriptorSet;
		writeOps[i].dstBinding = i;
		writeOps[i].dstArrayElement = 0;
		writeOps[i].descriptorCount = 1;
		writeOps[i].descriptorType = resourcesList[i].descriptorType;
		writeOps[i].pImageInfo = nullptr;
		writeOps[i].pBufferInfo = &resourcesList[i].descriptorBufferInfo;
		writeOps[i].pTexelBufferView = nullptr;
	}

	device.updateDescriptorSets
	(
		static_cast<uint32_t>(writeOps.size()), writeOps.data(), 0, nullptr
	);
}

// Creates a descriptor set layout
void mtd::DescriptorSetHandler::createDescriptorSetLayout
(
	const std::vector<vk::DescriptorSetLayoutBinding>& bindings
)
{
	resourcesList.resize(bindings.size());
	for(uint32_t i = 0; i < bindings.size(); i++)
	{
		resourcesList[i].descriptorType = bindings[i].descriptorType;
	}

	vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.flags = vk::DescriptorSetLayoutCreateFlags();
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutCreateInfo.pBindings = bindings.data();

	vk::Result result =
		device.createDescriptorSetLayout(&layoutCreateInfo, nullptr, &descriptorSetLayout);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create descriptor set layout. Vulkan result: %d", result);
}
