#include <pch.hpp>
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
void mtd::DescriptorSetHandler::defineDescriptorSetsAmount(uint32_t swappableSetsAmount)
{
	descriptorSets.resize(swappableSetsAmount);
	resourcesList.resize(swappableSetsAmount);
	for(std::vector<DescriptorResources>& setResourcesList: resourcesList)
		setResourcesList.resize(descriptorTypes.size());
}

// Creates a descriptor, assigning it to a set and returning the buffer write location
void mtd::DescriptorSetHandler::createDescriptorResources
(
	const Device& mtdDevice,
	vk::DeviceSize resourceSize,
	vk::BufferUsageFlags usageFlags,
	uint32_t swappableSetIndex,
	uint32_t binding
)
{
	assert
	(
		swappableSetIndex < resourcesList.size() &&
		binding < resourcesList[swappableSetIndex].size() &&
		"Descriptor out of bounds for resource creation."
	);

	DescriptorResources& resources = resourcesList[swappableSetIndex][binding];
	resources.descriptorBuffer = std::make_unique<GpuBuffer>
	(
		mtdDevice,
		resourceSize,
		usageFlags,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	resources.descriptorBufferInfo.buffer = resources.descriptorBuffer->getBuffer();
	resources.descriptorBufferInfo.offset = 0UL;
	resources.descriptorBufferInfo.range = resourceSize;
}

// Creates the resources for an image descriptor
void mtd::DescriptorSetHandler::createImageDescriptorResources
(
	uint32_t swappableSetIndex, uint32_t binding, const vk::DescriptorImageInfo& descriptorImageInfo
)
{
	assert
	(
		swappableSetIndex < resourcesList.size() &&
		binding < resourcesList[swappableSetIndex].size() &&
		"Descriptor out of bounds for image resource creation."
	);

	resourcesList[swappableSetIndex][binding].descriptorImageInfo = descriptorImageInfo;
}

// Updates the descriptor set write data
void mtd::DescriptorSetHandler::writeDescriptorSet(uint32_t swappableSetIndex)
{
	assert(swappableSetIndex < resourcesList.size() && "Swappable descriptor set not available to write data.");

	writeOps.resize(resourcesList[swappableSetIndex].size());
	for(uint32_t binding = 0; binding < resourcesList[swappableSetIndex].size(); binding++)
	{
		vk::DescriptorImageInfo* pImageInfo =
			descriptorTypes[binding] == vk::DescriptorType::eCombinedImageSampler
			? &resourcesList[swappableSetIndex][binding].descriptorImageInfo
			: nullptr;
		vk::DescriptorBufferInfo* pBufferInfo =
			descriptorTypes[binding] == vk::DescriptorType::eCombinedImageSampler
			? nullptr
			: &resourcesList[swappableSetIndex][binding].descriptorBufferInfo;

		writeOps[binding].dstSet = descriptorSets[swappableSetIndex];
		writeOps[binding].dstBinding = binding;
		writeOps[binding].dstArrayElement = 0;
		writeOps[binding].descriptorCount = 1;
		writeOps[binding].descriptorType = descriptorTypes[binding];
		writeOps[binding].pImageInfo = pImageInfo;
		writeOps[binding].pBufferInfo = pBufferInfo;
		writeOps[binding].pTexelBufferView = nullptr;
	}

	device.updateDescriptorSets(static_cast<uint32_t>(writeOps.size()), writeOps.data(), 0, nullptr);
}

// Updates the descriptor data
void mtd::DescriptorSetHandler::updateDescriptorData
(
	uint32_t swappableSetIndex, uint32_t binding, const void* data, vk::DeviceSize dataSize
) const
{
	assert
	(
		swappableSetIndex < resourcesList.size() &&
		binding < resourcesList[swappableSetIndex].size() &&
		"Descriptor out of bounds for data update."
	);

	const std::unique_ptr<GpuBuffer>& buffer = resourcesList[swappableSetIndex][binding].descriptorBuffer;
	buffer->copyMemoryToBuffer(dataSize, data);
}

// Creates a descriptor set layout
void mtd::DescriptorSetHandler::createDescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings)
{
	descriptorTypes.resize(bindings.size());
	for(uint32_t i = 0; i < bindings.size(); i++)
		descriptorTypes[i] = bindings[i].descriptorType;

	vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.flags = vk::DescriptorSetLayoutCreateFlags();
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutCreateInfo.pBindings = bindings.data();

	vk::Result result = device.createDescriptorSetLayout(&layoutCreateInfo, nullptr, &descriptorSetLayout);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create descriptor set layout. Vulkan result: %d", result);
}
