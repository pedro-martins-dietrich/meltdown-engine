#include "DescriptorPool.hpp"

#include "../../Utils/Logger.hpp"

mtd::DescriptorPool::DescriptorPool(const vk::Device& device) : device{device}
{
}

mtd::DescriptorPool::~DescriptorPool()
{
	device.destroyDescriptorPool(descriptorPool);
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

	vk::Result result =
		device.createDescriptorPool(&descriptorPoolCreateInfo, nullptr, &descriptorPool);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create descriptor pool. Vulkan result: %d", result);
}

// Allocates descriptor sets in the pool
void mtd::DescriptorPool::allocateDescriptorSet(DescriptorSetHandler& descriptorSetHandler) const
{
	vk::DescriptorSetAllocateInfo setAllocateInfo{};
	setAllocateInfo.descriptorPool = descriptorPool;
	setAllocateInfo.descriptorSetCount = descriptorSetHandler.getSetCount();
	setAllocateInfo.pSetLayouts = &descriptorSetHandler.getLayout();

	vk::Result result =
		device.allocateDescriptorSets(&setAllocateInfo, &descriptorSetHandler.getSet(0));
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to allocate descriptor set. Vulkan result: %d", result);
}
