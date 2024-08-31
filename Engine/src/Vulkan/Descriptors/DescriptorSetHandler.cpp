#include "DescriptorSetHandler.hpp"

#include "../../Utils/Logger.hpp"

mtd::DescriptorSetHandler::DescriptorSetHandler
(
	const vk::Device& device,
	const std::vector<vk::DescriptorSetLayoutBinding>& setLayoutBindings
) : device{device}
{
	createDescriptorSetLayout(setLayoutBindings);
}

mtd::DescriptorSetHandler::~DescriptorSetHandler()
{
	clearResources();

	device.destroyDescriptorSetLayout(descriptorSetLayout);
}

mtd::DescriptorSetHandler::DescriptorSetHandler(DescriptorSetHandler&& other) noexcept
	: device{other.device},
	descriptorSetLayout{std::move(other.descriptorSetLayout)},
	descriptorSets{std::move(other.descriptorSets)},
	descriptorTypes{std::move(other.descriptorTypes)},
	resourcesList{std::move(other.resourcesList)},
	writeOps{std::move(other.writeOps)}
{
	other.descriptorSetLayout = nullptr;
	for(vk::DescriptorSet& set: other.descriptorSets)
		set = nullptr;
}

// Defines how many descriptor sets can be associated with the descriptor set layout
void mtd::DescriptorSetHandler::defineDescriptorSetsAmount(uint32_t setsAmount)
{
	descriptorSets.resize(setsAmount);
	resourcesList.resize(setsAmount);
	for(std::vector<DescriptorResources>& setResourcesList: resourcesList)
		setResourcesList.resize(descriptorTypes.size());
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

// Creates the resources for an image descriptor
void mtd::DescriptorSetHandler::createImageDescriptorResources
(
	uint32_t setIndex, uint32_t resourceIndex, const vk::DescriptorImageInfo& descriptorImageInfo
)
{
	resourcesList[setIndex][resourceIndex].descriptorImageInfo = descriptorImageInfo;
}

// Updates the descriptor set data
void mtd::DescriptorSetHandler::writeDescriptorSet(uint32_t setIndex)
{
	writeOps.resize(resourcesList[setIndex].size());
	for(uint32_t i = 0; i < resourcesList[setIndex].size(); i++)
	{
		vk::DescriptorImageInfo* pImageInfo =
			descriptorTypes[i] == vk::DescriptorType::eCombinedImageSampler
			? &resourcesList[setIndex][i].descriptorImageInfo
			: nullptr;
		vk::DescriptorBufferInfo* pBufferInfo =
			descriptorTypes[i] == vk::DescriptorType::eCombinedImageSampler
			? nullptr
			: &resourcesList[setIndex][i].descriptorBufferInfo;

		writeOps[i].dstSet = descriptorSets[setIndex];
		writeOps[i].dstBinding = i;
		writeOps[i].dstArrayElement = 0;
		writeOps[i].descriptorCount = 1;
		writeOps[i].descriptorType = descriptorTypes[i];
		writeOps[i].pImageInfo = pImageInfo;
		writeOps[i].pBufferInfo = pBufferInfo;
		writeOps[i].pTexelBufferView = nullptr;
	}

	device.updateDescriptorSets
	(
		static_cast<uint32_t>(writeOps.size()), writeOps.data(), 0, nullptr
	);
}

// Deletes all resources in GPU memory
void mtd::DescriptorSetHandler::clearResources()
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
}

// Creates a descriptor set layout
void mtd::DescriptorSetHandler::createDescriptorSetLayout
(
	const std::vector<vk::DescriptorSetLayoutBinding>& bindings
)
{
	descriptorTypes.resize(bindings.size());
	for(uint32_t i = 0; i < bindings.size(); i++)
	{
		descriptorTypes[i] = bindings[i].descriptorType;
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
