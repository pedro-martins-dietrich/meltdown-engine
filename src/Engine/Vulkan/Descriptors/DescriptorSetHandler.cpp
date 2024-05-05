#include "DescriptorSetHandler.hpp"

#include "../../Utils/Logger.hpp"

mtd::DescriptorSetHandler::DescriptorSetHandler
(
	const vk::Device& device,
	const std::vector<vk::DescriptorSetLayoutBinding>& setLayoutBindings,
	uint32_t maxSets
) : device{device}
{
	descriptorSets.resize(maxSets);
	resourcesList.resize(maxSets);
	createDescriptorSetLayout(setLayoutBindings);
}

mtd::DescriptorSetHandler::~DescriptorSetHandler()
{
	for(std::vector<DescriptorResources>& setResourcesList: resourcesList)
	{
		for(DescriptorResources& resources: setResourcesList)
		{
			if(resources.descriptorBuffer.bufferMemory)
			{
				device.unmapMemory(resources.descriptorBuffer.bufferMemory);
				device.freeMemory(resources.descriptorBuffer.bufferMemory);
				device.destroyBuffer(resources.descriptorBuffer.buffer);
			}
		}
	}

	device.destroyDescriptorSetLayout(descriptorSetLayout);
}

mtd::DescriptorSetHandler::DescriptorSetHandler(DescriptorSetHandler&& other) noexcept
	: device{other.device},
	descriptorSetLayout{std::move(other.descriptorSetLayout)},
	descriptorSets{std::move(other.descriptorSets)},
	resourcesList{std::move(other.resourcesList)}
{
	other.descriptorSetLayout = nullptr;
	for(vk::DescriptorSet& set: other.descriptorSets)
		set = nullptr;
}

// Creates a descriptor and assings it to a descriptor set
void mtd::DescriptorSetHandler::createDescriptorResources
(
	const Device& mtdDevice,
	vk::DeviceSize resourceSize,
	vk::BufferUsageFlags usageFlags,
	uint32_t setIndex,
	uint32_t resourceIndex
)
{
	DescriptorResources& resources = resourcesList[setIndex][resourceIndex];
	Memory::createBuffer
	(
		mtdDevice,
		resources.descriptorBuffer,
		resourceSize,
		usageFlags,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	resources.descriptorBufferWriteLocation = device.mapMemory
	(
		resources.descriptorBuffer.bufferMemory, 0UL, resourceSize
	);
	resources.descriptorBufferInfo.buffer = resources.descriptorBuffer.buffer;
	resources.descriptorBufferInfo.offset = 0UL;
	resources.descriptorBufferInfo.range = resourceSize;
}

// Updates the descriptor set data
void mtd::DescriptorSetHandler::writeDescriptorSet(uint32_t setIndex)
{
	writeOps.resize(resourcesList[setIndex].size());
	for(uint32_t i = 0; i < resourcesList[setIndex].size(); i++)
	{
		writeOps[i].dstSet = descriptorSets[setIndex];
		writeOps[i].dstBinding = i;
		writeOps[i].dstArrayElement = 0;
		writeOps[i].descriptorCount = 1;
		writeOps[i].descriptorType = resourcesList[setIndex][i].descriptorType;
		writeOps[i].pImageInfo = nullptr;
		writeOps[i].pBufferInfo = &resourcesList[setIndex][i].descriptorBufferInfo;
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
	for(std::vector<DescriptorResources>& setResourcesList: resourcesList)
	{
		setResourcesList.resize(bindings.size());
		for(uint32_t i = 0; i < bindings.size(); i++)
		{
			setResourcesList[i].descriptorType = bindings[i].descriptorType;
		}
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
