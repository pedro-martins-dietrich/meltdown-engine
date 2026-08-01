#include <pch.hpp>
#include "ResourceManager.hpp"

#include "../Utils/Logger.hpp"
#include "../Vulkan/EnumMapping/EnumMapping.hpp"

namespace mtd
{
    using ResourceIdConstIterator = std::unordered_map<std::string, ResourceID>::const_iterator;
    using BufferIterator = std::unordered_map<ResourceID, GpuBuffer>::iterator;
    using BufferConstIterator = std::unordered_map<ResourceID, GpuBuffer>::const_iterator;
    using ImageIterator = std::unordered_map<ResourceID, Image>::iterator;
    using ImageConstIterator = std::unordered_map<ResourceID, Image>::const_iterator;
}

mtd::ResourceManager::ResourceManager(const Device& device, UIntVec2 windowResolution)
    : device{device}, windowResolution{windowResolution}
{}

mtd::ResourceID mtd::ResourceManager::getResourceID(std::string_view resourceName) const
{
    ResourceIdConstIterator ni = nameIdMap.find(resourceName.data());
    if(ni == nameIdMap.cend()) return 0U;

    return ni->second;
}

mtd::ResourceID mtd::ResourceManager::createBuffer
(
    std::string resourceName, GpuBufferType type, GpuMemoryUsage memoryUsage, uint64_t bufferSize, const void* pData
)
{
    if(resourceName.length() == 0UL)
    {
        LOG_ERROR("Valid name for the GPU resource has not been provided. GPU buffer will not be created.");
        return 0U;
    }

    if(memoryUsage == GpuMemoryUsage::Auto)
        memoryUsage = EnumMapping::defaultMemoryUsage(type);
    vk::BufferUsageFlags bufferUsage = EnumMapping::getBufferUsage(type);
    vk::MemoryPropertyFlags memoryProperties = EnumMapping::getMemoryProperties(memoryUsage);

    if(bufferSize == 0UL)
        buffers.emplace(nextID, GpuBuffer{device, bufferUsage, memoryProperties});
    else
    {
        buffers.emplace(nextID, GpuBuffer{device, bufferSize, bufferUsage, memoryProperties});
        if(pData)
            buffers.at(nextID).copyMemoryToBuffer(bufferSize, pData);
    }


    nameIdMap.emplace(std::move(resourceName), nextID);

    return nextID++;
}

mtd::ResourceID mtd::ResourceManager::createImage
(
    std::string resourceName,
    UIntVec2 imageDimensions,
    vk::Format imageFormat,
    vk::ImageUsageFlags usage,
    Vec2 windowResolutionRatio,
    SamplerType samplerType,
    vk::ImageTiling tiling,
    vk::ImageAspectFlags aspect,
    vk::ImageViewType viewType,
    vk::MemoryPropertyFlags memoryProperties
)
{
    if(resourceName.length() == 0UL)
    {
        LOG_ERROR("Valid name for the GPU resource has not been provided. GPU image will not be created.");
        return 0U;
    }

    if(windowResolutionRatio.x > 0.0f)
        imageDimensions.x = static_cast<uint32_t>(windowResolutionRatio.x * windowResolution.x);
    if(windowResolutionRatio.y > 0.0f)
        imageDimensions.y = static_cast<uint32_t>(windowResolutionRatio.y * windowResolution.y);

    images.emplace
    (
        nextID,
        Image
        {
            device,
            imageDimensions,
            imageFormat,
            tiling,
            usage,
            memoryProperties,
            aspect,
            viewType,
            samplerType,
            windowResolutionRatio
        }
    );

    nameIdMap.emplace(std::move(resourceName), nextID);

    return nextID++;
}

bool mtd::ResourceManager::updateBufferData
(
    ResourceID id, vk::DeviceSize copySize, const void* srcData, vk::DeviceSize bufferOffset
)
{
    BufferIterator b = buffers.find(id);
    if(b == buffers.cend()) return false;

    b->second.copyMemoryToBuffer(copySize, srcData, bufferOffset);
    return true;
}

bool mtd::ResourceManager::transitionImageLayout
(
    ResourceID id,
    vk::CommandBuffer commandBuffer,
    vk::ImageLayout newLayout,
    vk::PipelineStageFlags srcStage,
    vk::PipelineStageFlags dstStage
) const
{
    ImageConstIterator i = images.find(id);
    if(i == images.cend()) return false;

    i->second.transitionImageLayout(commandBuffer, newLayout, srcStage, dstStage);
    return true;
}

void mtd::ResourceManager::updateWindowResolutionLinkedImages(UIntVec2 newWindowResolution)
{
    windowResolution = newWindowResolution;

    for(auto& [id, image]: images)
    {
        Vec2 ratio = image.getWindowResolutionRatio();
        if(ratio.x <= 0.0f && ratio.y <= 0.0f) continue;

        image.resize
        ({
            static_cast<uint32_t>(ratio.x * windowResolution.x),
            static_cast<uint32_t>(ratio.y * windowResolution.y)
        });
    }
}

bool mtd::ResourceManager::deleteResource(ResourceID id)
{
    return (buffers.erase(id) > 0UL) || (images.erase(id) > 0UL);
}

void mtd::ResourceManager::clearResources()
{
    buffers.clear();
    images.clear();
    nextID = 1U;
}

bool mtd::ResourceManager::fetchDescriptorBufferInfo(ResourceID id, vk::DescriptorBufferInfo& info) const
{
    BufferConstIterator b = buffers.find(id);
    if(b == buffers.cend()) return false;

    b->second.updateDescriptorInfo(info);
    return true;
}

bool mtd::ResourceManager::fetchDescriptorImageInfo(ResourceID id, vk::DescriptorImageInfo& info) const
{
    ImageConstIterator i = images.find(id);
    if(i == images.cend()) return false;

    i->second.updateDescriptorInfo(info);
    return true;
}
