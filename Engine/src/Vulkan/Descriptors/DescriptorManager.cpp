#include <pch.hpp>
#include "DescriptorManager.hpp"

#include "../EnumMapping/EnumMapping.hpp"
#include "../../Utils/Logger.hpp"

mtd::DescriptorManager::DescriptorManager(const Device& mtdDevice, const ResourceManager& resourceManager)
    : mtdDevice{mtdDevice}, resourceManager{resourceManager}, pool{nullptr}
{}

mtd::DescriptorManager::~DescriptorManager()
{
    if(pool)
        mtdDevice.getDevice().destroyDescriptorPool(pool);
}

vk::DescriptorSetLayout mtd::DescriptorManager::getLayout(DescriptorSetID id) const
{
    if(id < sets.size())
        return sets[id].getLayout();

    LOG_ERROR("Inexistent descriptor set ID: %d.", id);
    return nullptr;
}

vk::DescriptorSet mtd::DescriptorManager::getSet(DescriptorSetID id) const
{
    if(id < sets.size())
        return sets[id].getSet();

    LOG_ERROR("Inexisting descriptor set ID: %d.", id);
    return nullptr;
}

mtd::DescriptorSetID mtd::DescriptorManager::createSet(const std::vector<DescriptorInfo>& descriptorInfos)
{
    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{descriptorInfos.size()};
    for(uint32_t i = 0U; i < descriptorInfos.size(); i++)
    {
        setLayoutBindings[i].binding = i;
        setLayoutBindings[i].descriptorType = EnumMapping::getDescriptorType(descriptorInfos[i].descriptorType);
        setLayoutBindings[i].descriptorCount = descriptorInfos[i].descriptorCount;
        setLayoutBindings[i].stageFlags = EnumMapping::getShaderStage(descriptorInfos[i].shaderStage);
        setLayoutBindings[i].pImmutableSamplers = nullptr;

        totalDescriptorTypeCount[setLayoutBindings[i].descriptorType] += setLayoutBindings[i].descriptorCount;
    }
    sets.emplace_back(mtdDevice.getDevice(), setLayoutBindings);

    for(uint32_t i = 0U; i < descriptorInfos.size(); i++)
    {
        ResourceID resourceID = resourceManager.getResourceID(descriptorInfos[i].resourceName);
        vk::DescriptorBufferInfo bufferInfo;
        if(resourceManager.fetchDescriptorBufferInfo(resourceID, bufferInfo))
            sets.back().assignBuffer(i, bufferInfo);
    }

    return static_cast<DescriptorSetID>(sets.size() - 1UL);
}

void mtd::DescriptorManager::allocate()
{
    createPool();
    if(!pool) return;

    std::vector<vk::DescriptorSetLayout> layouts{sets.size()};
    std::vector<vk::DescriptorSet> descSets{sets.size()};
    for(uint32_t i = 0U; i < sets.size(); i++)
    {
        layouts[i] = sets[i].getLayout();
        descSets[i] = sets[i].getSet();
    }

    vk::DescriptorSetAllocateInfo setAllocateInfo{};
	setAllocateInfo.descriptorPool = pool;
	setAllocateInfo.descriptorSetCount = layouts.size();
	setAllocateInfo.pSetLayouts = layouts.data();
	setAllocateInfo.pNext = nullptr;

	vk::Result result = mtdDevice.getDevice().allocateDescriptorSets(&setAllocateInfo, descSets.data());
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to allocate descriptor set. Vulkan result: %d", result);

    for(uint32_t i = 0U; i < sets.size(); i++)
    {
        sets[i].defineDescriptorSet(descSets[i]);
        sets[i].writeDescriptorSet();
    }
}

void mtd::DescriptorManager::clear()
{
    if(pool)
        mtdDevice.getDevice().destroyDescriptorPool(pool);
    pool = nullptr;

    totalDescriptorTypeCount.clear();

    sets.clear();
}

void mtd::DescriptorManager::createPool()
{
    assert(!pool && "The descriptor pool has already been created.");

    uint32_t maxSets = 0U;
    std::vector<vk::DescriptorPoolSize> poolSizes;
    poolSizes.reserve(totalDescriptorTypeCount.size());

    for(const auto& [type, count]: totalDescriptorTypeCount)
    {
        if(count == 0U) continue;
        poolSizes.emplace_back(type, count);
        maxSets += count;
    }

    if(maxSets == 0U)
    {
        LOG_VERBOSE("No descriptors being used. Descriptor pool will not be created.");
        return;
    }

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlags();
	descriptorPoolCreateInfo.maxSets = maxSets;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

	vk::Result result = mtdDevice.getDevice().createDescriptorPool(&descriptorPoolCreateInfo, nullptr, &pool);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create descriptor pool. Vulkan result: %d", result);
}
