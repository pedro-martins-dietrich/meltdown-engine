#include <pch.hpp>
#include "DescriptorSetHandler.hpp"

#include "../../Utils/Logger.hpp"

mtd::DescriptorSetHandler::DescriptorSetHandler
(
	const vk::Device& device,
	const std::vector<vk::DescriptorSetLayoutBinding>& setLayoutBindings,
	const vk::DescriptorSetLayoutBindingFlagsCreateInfo* pFlags
) : device{device}, descriptorSetLayout{nullptr}, descriptorSet{nullptr}
{
	createDescriptorSetLayout(setLayoutBindings, pFlags);
}

mtd::DescriptorSetHandler::~DescriptorSetHandler()
{
	device.destroyDescriptorSetLayout(descriptorSetLayout);
}

mtd::DescriptorSetHandler::DescriptorSetHandler(DescriptorSetHandler&& other) noexcept
	: device{other.device},
	descriptorSetLayout{std::move(other.descriptorSetLayout)},
	descriptorSet{std::move(other.descriptorSet)},
	writeOperations{std::move(other.writeOperations)},
	resourcesList{std::move(other.resourcesList)}
{
	other.descriptorSetLayout = nullptr;
	other.descriptorSet = nullptr;
}

void mtd::DescriptorSetHandler::createDescriptorResources
(
	const Device& mtdDevice, vk::DeviceSize resourceSize, vk::BufferUsageFlags usageFlags, uint32_t binding
)
{
	assert(binding < resourcesList.size() && "Descriptor out of bounds for resource creation.");

	DescriptorResources& resources = resourcesList[binding];
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

	writeOperations[binding].dstSet = descriptorSet;
	writeOperations[binding].pBufferInfo = &(resources.descriptorBufferInfo);
}

void mtd::DescriptorSetHandler::createImageDescriptorResources
(
	uint32_t binding, const vk::DescriptorImageInfo& descriptorImageInfo
)
{
	assert(binding < resourcesList.size() && "Descriptor out of bounds for image resource creation.");

	resourcesList[binding].descriptorImagesInfo = {descriptorImageInfo};

	writeOperations[binding].dstSet = descriptorSet;
	writeOperations[binding].pImageInfo = resourcesList[binding].descriptorImagesInfo.data();
}

void mtd::DescriptorSetHandler::createImagesDescriptorResources
(
	uint32_t binding, std::vector<vk::DescriptorImageInfo>& descriptorImagesInfo
)
{
	assert(binding < resourcesList.size() && "Descriptor out of bounds for image resource creation.");

	resourcesList[binding].descriptorImagesInfo = std::move(descriptorImagesInfo);

	writeOperations[binding].dstSet = descriptorSet;
	writeOperations[binding].descriptorCount
		= static_cast<uint32_t>(resourcesList[binding].descriptorImagesInfo.size());
	writeOperations[binding].pImageInfo = resourcesList[binding].descriptorImagesInfo.data();
}

void mtd::DescriptorSetHandler::assignExternalResourcesToDescriptor
(
	uint32_t binding, const GpuBuffer& buffer, const void* pNext
)
{
	assert(binding < resourcesList.size() && "Descriptor out of bounds for resource assignment.");

	vk::DescriptorBufferInfo& descriptorBufferInfo = resourcesList[binding].descriptorBufferInfo;
	descriptorBufferInfo.buffer = buffer.getBuffer();
	descriptorBufferInfo.offset = 0UL;
	descriptorBufferInfo.range = buffer.getSize();

	writeOperations[binding].dstSet = descriptorSet;
	writeOperations[binding].pBufferInfo = &descriptorBufferInfo;
	writeOperations[binding].pNext = pNext;
}

void mtd::DescriptorSetHandler::assignBuffer
(
	uint32_t binding, const vk::DescriptorBufferInfo& bufferInfo, const void* pNext
)
{
	assert(binding < resourcesList.size() && "Descriptor out of bounds for GPU buffer assignment.");

	resourcesList[binding].descriptorBufferInfo = bufferInfo;

	writeOperations[binding].dstSet = descriptorSet;
	writeOperations[binding].pBufferInfo = &(resourcesList[binding].descriptorBufferInfo);
	writeOperations[binding].pNext = pNext;
}

void mtd::DescriptorSetHandler::defineDescriptorSet(vk::DescriptorSet set)
{
	assert(!descriptorSet && "The descriptor set is already defined and cannot be overwritten.");
	descriptorSet = set;
}

void mtd::DescriptorSetHandler::writeDescriptorSet()
{
	for(vk::WriteDescriptorSet& writeOp: writeOperations)
		writeOp.dstSet = descriptorSet;
	device.updateDescriptorSets(static_cast<uint32_t>(writeOperations.size()), writeOperations.data(), 0U, nullptr);
}

void mtd::DescriptorSetHandler::writeDescriptor(uint32_t binding)
{
	assert(binding < writeOperations.size() && "Descriptor binding out of bounds for update.");

	writeOperations[binding].dstSet = descriptorSet;
	device.updateDescriptorSets(1U, &(writeOperations[binding]), 0U, nullptr);
}

void mtd::DescriptorSetHandler::updateDescriptorData(uint32_t binding, const void* data, vk::DeviceSize dataSize) const
{
	assert(binding < resourcesList.size() && "Descriptor out of bounds for data update.");
	assert((resourcesList[binding].descriptorBuffer.get() != nullptr) && "Descriptor buffer is not updatable.");

	const std::unique_ptr<GpuBuffer>& buffer = resourcesList[binding].descriptorBuffer;
	buffer->copyMemoryToBuffer(dataSize, data);
}

void mtd::DescriptorSetHandler::createDescriptorSetLayout
(
	const std::vector<vk::DescriptorSetLayoutBinding>& bindings,
	const vk::DescriptorSetLayoutBindingFlagsCreateInfo* pFlags
)
{
	resourcesList.resize(bindings.size());
	writeOperations.resize(bindings.size());
	for(uint32_t bindingIndex = 0U; bindingIndex < bindings.size(); bindingIndex++)
	{
		writeOperations[bindingIndex].dstSet = descriptorSet;
		writeOperations[bindingIndex].dstBinding = bindingIndex;
		writeOperations[bindingIndex].dstArrayElement = 0U;
		writeOperations[bindingIndex].descriptorCount = bindings[bindingIndex].descriptorCount;
		writeOperations[bindingIndex].descriptorType = bindings[bindingIndex].descriptorType;
		writeOperations[bindingIndex].pImageInfo = nullptr;
		writeOperations[bindingIndex].pBufferInfo = nullptr;
		writeOperations[bindingIndex].pTexelBufferView = nullptr;
		writeOperations[bindingIndex].pNext = nullptr;
	}

	vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.flags = vk::DescriptorSetLayoutCreateFlags();
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutCreateInfo.pBindings = bindings.data();
	layoutCreateInfo.pNext = pFlags;

	vk::Result result = device.createDescriptorSetLayout(&layoutCreateInfo, nullptr, &descriptorSetLayout);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create descriptor set layout. Vulkan result: %d", result);
}
