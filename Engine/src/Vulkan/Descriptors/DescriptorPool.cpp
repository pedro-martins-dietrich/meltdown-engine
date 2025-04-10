#include <pch.hpp>
#include "DescriptorPool.hpp"

#include "../../Utils/Logger.hpp"

mtd::DescriptorPool::DescriptorPool(const vk::Device& device)
	: device{device}, isClear{true}
{}

mtd::DescriptorPool::~DescriptorPool()
{
	clear();
}

// Creates a descriptor pool
void mtd::DescriptorPool::createDescriptorPool
(
	const std::vector<PoolSizeData>& poolSizesInfo, vk::DescriptorPoolCreateFlags flags
)
{
	uint32_t maxSets = 0;

	std::vector<vk::DescriptorPoolSize> poolSizes{poolSizesInfo.size()};
	for(uint32_t i = 0; i < poolSizesInfo.size(); i++)
	{
		poolSizes[i].type = poolSizesInfo[i].descriptorType;
		poolSizes[i].descriptorCount = poolSizesInfo[i].descriptorCount;
		maxSets += poolSizesInfo[i].descriptorCount;
	}

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.flags = flags;
	descriptorPoolCreateInfo.maxSets = maxSets;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

	isClear = false;
	vk::Result result = device.createDescriptorPool(&descriptorPoolCreateInfo, nullptr, &descriptorPool);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create descriptor pool. Vulkan result: %d", result);
}

// Allocates descriptor sets in the pool
void mtd::DescriptorPool::allocateDescriptorSet
(
	DescriptorSetHandler& descriptorSetHandler,
	const void* pExtraAllocateInfo
) const
{
	std::vector<vk::DescriptorSetLayout> layouts{descriptorSetHandler.getSetCount(), descriptorSetHandler.getLayout()};

	vk::DescriptorSetAllocateInfo setAllocateInfo{};
	setAllocateInfo.descriptorPool = descriptorPool;
	setAllocateInfo.descriptorSetCount = descriptorSetHandler.getSetCount();
	setAllocateInfo.pSetLayouts = layouts.data();
	setAllocateInfo.pNext = pExtraAllocateInfo;

	vk::Result result = device.allocateDescriptorSets(&setAllocateInfo, descriptorSetHandler.getSets().data());
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to allocate descriptor set. Vulkan result: %d", result);
}

// Clears the descriptor pool and its sets
void mtd::DescriptorPool::clear()
{
	if(isClear) return;

	isClear = true;
	device.destroyDescriptorPool(descriptorPool);
}
