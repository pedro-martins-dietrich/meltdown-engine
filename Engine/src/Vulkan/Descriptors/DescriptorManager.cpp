#include <pch.hpp>
#include "DescriptorManager.hpp"

#include "../EnumMapping/EnumMapping.hpp"
#include "../../Utils/Logger.hpp"

namespace mtd
{
    using ResourceDescriptorsMapIterator
        = std::unordered_map<ResourceID, std::vector<DescriptorIdentifier>>::const_iterator;

    // Checks if the descriptor type corresponds to a GPU buffer
    static bool isBufferDescriptor(vk::DescriptorType type);
    // Checks if the descriptor type corresponds to a GPU image
    static bool isImageDescriptor(vk::DescriptorType type);
}

mtd::DescriptorManager::DescriptorManager(const Device& mtdDevice, const ResourceManager& resourceManager)
    : mtdDevice{mtdDevice}, resourceManager{resourceManager}, pool{nullptr}
{}

mtd::DescriptorManager::~DescriptorManager()
{
    clear();
}

vk::DescriptorSetLayout mtd::DescriptorManager::getLayout(DescriptorLayoutID id) const
{
    if(id < layouts.size())
        return layouts[id].layout;

    LOG_ERROR("Inexistent descriptor layout ID: %d.", id);
    return nullptr;
}

vk::DescriptorSet mtd::DescriptorManager::getSet(DescriptorSetID id) const
{
    if(id < sets.size())
        return sets[id].set;

    LOG_ERROR("Inexisting descriptor set ID: %d.", id);
    return nullptr;
}

mtd::DescriptorLayoutID mtd::DescriptorManager::createLayout
(
    const std::vector<DescriptorLayoutBindingInfo>& bindingInfos
)
{
    DescriptorLayoutData layoutData;
    layoutData.bindings.resize(bindingInfos.size());

    std::vector<vk::DescriptorSetLayoutBinding> bindings(bindingInfos.size());
    for(uint32_t i = 0U; i < bindingInfos.size(); i++)
    {
        bindings[i].binding = i;
        bindings[i].descriptorType = EnumMapping::getDescriptorType(bindingInfos[i].type);
        bindings[i].descriptorCount = bindingInfos[i].count;
        bindings[i].stageFlags = EnumMapping::getShaderStage(bindingInfos[i].shaderStage);
        bindings[i].pImmutableSamplers = nullptr;

        layoutData.bindings[i].type = bindings[i].descriptorType;
        layoutData.bindings[i].count = bindings[i].descriptorCount;

        totalDescriptorTypeCount[bindings[i].descriptorType] += bindings[i].descriptorCount;
        totalBindingsCount++;
    }

    vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.flags = vk::DescriptorSetLayoutCreateFlags();
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutCreateInfo.pBindings = bindings.data();
	layoutCreateInfo.pNext = nullptr;

	vk::Result result = mtdDevice.getDevice().createDescriptorSetLayout(&layoutCreateInfo, nullptr, &(layoutData.layout));
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create descriptor set layout. Vulkan result: %d", result);

    DescriptorLayoutID layoutID = static_cast<DescriptorLayoutID>(layouts.size());
    layouts.emplace_back(layoutData);

    return layoutID;
}

mtd::DescriptorSetID mtd::DescriptorManager::allocateSet(DescriptorSetInfo& descriptorSetInfo, uint32_t count)
{
    assert(pool && "The descriptor pool must be created before the descriptor sets.");

    if(descriptorSetInfo.layoutID >= layouts.size())
    {
        LOG_ERROR("Invalid descriptor layout ID: %d. Cannot create descriptor set.");
        return UINT32_MAX;
    }

    const DescriptorLayoutData& layout = layouts[descriptorSetInfo.layoutID];

    DescriptorSetData setData;
    setData.layoutID = descriptorSetInfo.layoutID;
    setData.resources = std::move(descriptorSetInfo.resources);

    std::vector<vk::DescriptorSetLayout> vulkanLayouts(count, layout.layout);
    std::vector<vk::DescriptorSet> vulkanSets(count);

    vk::DescriptorSetAllocateInfo setAllocateInfo{};
	setAllocateInfo.descriptorPool = pool;
	setAllocateInfo.descriptorSetCount = count;
	setAllocateInfo.pSetLayouts = vulkanLayouts.data();
	setAllocateInfo.pNext = nullptr;

    vk::Result result = mtdDevice.getDevice().allocateDescriptorSets(&setAllocateInfo, vulkanSets.data());
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to allocate descriptor sets. Vulkan result: %d", result);

    DescriptorSetID setID = static_cast<DescriptorSetID>(sets.size());
    for(size_t i = 0; i < count; i++)
    {
        setData.set = vulkanSets[i];
        sets.emplace_back(setData);

        for(uint32_t bindingIndex = 0U; bindingIndex < setData.resources.size(); bindingIndex++)
        {
            if(isBufferDescriptor(layout.bindings[bindingIndex].type))
                totalBufferResourcesCount += layout.bindings[bindingIndex].count;
            else if(isImageDescriptor(layout.bindings[bindingIndex].type))
                totalImageResourcesCount += layout.bindings[bindingIndex].count;

            DescriptorIdentifier descriptorIdentity{setID, bindingIndex};
            for(ResourceID resourceID: setData.resources[bindingIndex])
            {
                ResourceDescriptorsMapIterator it = resourceDescriptorsMap.find(resourceID);
                if(it == resourceDescriptorsMap.cend())
                    resourceDescriptorsMap[resourceID] = {descriptorIdentity};
                else
                    resourceDescriptorsMap.at(resourceID).emplace_back(descriptorIdentity);
            }
        }
    }

    return setID;
}

void mtd::DescriptorManager::writeAll()
{
    if(sets.size() == 0) return;

    assert(totalBindingsCount > 0U && "Cannot write descriptors if none have been allocated.");

    std::vector<vk::WriteDescriptorSet> writeOperations(totalBindingsCount);
    size_t operationIndex = 0;

    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    std::vector<vk::DescriptorImageInfo> imageInfos;
    bufferInfos.reserve(totalBufferResourcesCount);
    imageInfos.reserve(totalImageResourcesCount);

    for(DescriptorSetID setID = 0U; setID < sets.size(); setID++)
    {
        for(uint32_t binding = 0U; binding < sets[setID].resources.size(); binding++)
        {
            assert(operationIndex < writeOperations.size() && "Descriptor write info index out of bounds.");
            buildWriteOperation(setID, binding, writeOperations[operationIndex], bufferInfos, imageInfos);
            operationIndex++;
        }
    }

    mtdDevice.getDevice().updateDescriptorSets
    (
        static_cast<uint32_t>(writeOperations.size()), writeOperations.data(), 0U, nullptr
    );
}

void mtd::DescriptorManager::write(DescriptorSetID setID, uint32_t binding)
{
    assert(setID < sets.size() && "Descriptor Set ID out of bounds for write operation.");
    assert(binding < sets[setID].resources.size() && "Descriptor set binding out of bounds for write operation.");

    vk::WriteDescriptorSet writeOperation;
    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    std::vector<vk::DescriptorImageInfo> imageInfos;
    buildWriteOperation(setID, binding, writeOperation, bufferInfos, imageInfos);
    mtdDevice.getDevice().updateDescriptorSets(1U, &writeOperation, 0U, nullptr);
}

void mtd::DescriptorManager::updateResourceDescriptors(ResourceID resourceID)
{
    ResourceDescriptorsMapIterator it = resourceDescriptorsMap.find(resourceID);
    if(it == resourceDescriptorsMap.cend()) return;

    for(DescriptorIdentifier descriptorIdentity: it->second)
        write(descriptorIdentity.setID, descriptorIdentity.binding);
}

void mtd::DescriptorManager::clear()
{
    if(pool)
        mtdDevice.getDevice().destroyDescriptorPool(pool);
    pool = nullptr;

    totalBindingsCount = 0;
    totalBufferResourcesCount = 0;
    totalImageResourcesCount = 0;
    totalDescriptorTypeCount.clear();

    for(DescriptorLayoutData& layoutData: layouts)
        mtdDevice.getDevice().destroyDescriptorSetLayout(layoutData.layout);
    layouts.clear();
    sets.clear();
}

void mtd::DescriptorManager::createPool()
{
    assert(!pool && "The descriptor pool has already been created.");

    std::vector<vk::DescriptorPoolSize> poolSizes;
    poolSizes.reserve(totalDescriptorTypeCount.size());

    for(const auto& [type, count]: totalDescriptorTypeCount)
    {
        if(count == 0U) continue;
        poolSizes.emplace_back(type, count);
    }

    if(poolSizes.size() == 0U)
    {
        LOG_VERBOSE("No descriptors being used. Descriptor pool will not be created.");
        return;
    }

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlags();
	descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(layouts.size());
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

	vk::Result result = mtdDevice.getDevice().createDescriptorPool(&descriptorPoolCreateInfo, nullptr, &pool);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create descriptor pool. Vulkan result: %d", result);
}

void mtd::DescriptorManager::buildWriteOperation
(
    DescriptorSetID setID, uint32_t binding, vk::WriteDescriptorSet& writeOperation,
    std::vector<vk::DescriptorBufferInfo>& bufferInfos, std::vector<vk::DescriptorImageInfo>& imageInfos
) const
{
    assert(sets[setID].layoutID < layouts.size() && "Invalid descriptor layout ID for write operation.");
    DescriptorLayoutID layoutID = sets[setID].layoutID;
    
    assert(binding < layouts[layoutID].bindings.size() && "Binding out of bounds for descriptor layout.");
    const DescriptorLayoutBindingData& bindingData = layouts[layoutID].bindings[binding];
    const std::vector<ResourceID>& descriptorResources = sets[setID].resources[binding];

    writeOperation.dstSet = sets[setID].set;
    writeOperation.dstBinding = binding;
    writeOperation.dstArrayElement = 0U;
    writeOperation.descriptorCount = bindingData.count;
    writeOperation.descriptorType = bindingData.type;
    writeOperation.pImageInfo = nullptr;
    writeOperation.pBufferInfo = nullptr;
    writeOperation.pTexelBufferView = nullptr;
    writeOperation.pNext = nullptr;

    if(isBufferDescriptor(bindingData.type))
    {
        size_t offset = bufferInfos.size();
        for(ResourceID resourceID: descriptorResources)
        {
            vk::DescriptorBufferInfo bufferInfo;
            if(resourceManager.fetchDescriptorBufferInfo(resourceID, bufferInfo))
                bufferInfos.emplace_back(bufferInfo);
        }
        writeOperation.pBufferInfo = &(bufferInfos[offset]);
    }
    else if(isImageDescriptor(bindingData.type))
    {
        size_t offset = imageInfos.size();
        for(ResourceID resourceID: descriptorResources)
        {
            vk::DescriptorImageInfo imageInfo;
            if(resourceManager.fetchDescriptorImageInfo(resourceID, imageInfo))
                imageInfos.emplace_back(imageInfo);
        }
        writeOperation.pImageInfo = &(imageInfos[offset]);
    }
    else
    {
        LOG_ERROR
        (
            "Invalid resource type for descriptor write operation.\nLayout ID: %d\tSet ID: %d\tBinding: %d\tType: %d",
            layoutID, setID, binding, static_cast<uint32_t>(bindingData.type)
        );
    }
}

bool mtd::isBufferDescriptor(vk::DescriptorType type)
{
    switch(type)
    {
        case vk::DescriptorType::eUniformBuffer:
        case vk::DescriptorType::eStorageBuffer:
        case vk::DescriptorType::eUniformBufferDynamic:
        case vk::DescriptorType::eStorageBufferDynamic:
            return true;
        default:
            return false;
    }
    return false;
}

bool mtd::isImageDescriptor(vk::DescriptorType type)
{
    switch(type)
    {
        case vk::DescriptorType::eSampler:
        case vk::DescriptorType::eCombinedImageSampler:
        case vk::DescriptorType::eSampledImage:
        case vk::DescriptorType::eStorageImage:
        case vk::DescriptorType::eInputAttachment:
            return true;
        default:
            return false;
    }
    return false;
}
