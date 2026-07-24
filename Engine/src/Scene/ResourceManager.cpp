#include <pch.hpp>
#include "ResourceManager.hpp"

namespace mtd
{
    using ResourceIterator = std::unordered_map<ResourceID, Resource>::iterator;
    using ResourceConstIterator = std::unordered_map<ResourceID, Resource>::const_iterator;
}

mtd::ResourceManager::ResourceManager(const Device& device) : device{device}
{}

mtd::ResourceID mtd::ResourceManager::createBuffer
(
    vk::BufferUsageFlags bufferUsage, vk::MemoryPropertyFlags memoryProperties, vk::DeviceSize bufferSize
)
{
    if(bufferSize == 0U)
        resources.emplace(nextID, GpuBuffer{device, bufferUsage, memoryProperties});
    else
        resources.emplace(nextID, GpuBuffer{device, bufferSize, bufferUsage, memoryProperties});
    return nextID++;
}

mtd::ResourceID mtd::ResourceManager::createImage
(
    UIntVec2 imageDimensions,
    vk::Format imageFormat,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags memoryProperties,
    vk::ImageAspectFlags aspect,
    vk::ImageViewType viewType,
    SamplerType samplerType
)
{
    resources.emplace
    (
        nextID,
        Image{device, imageDimensions, imageFormat, tiling, usage, memoryProperties, aspect, viewType, samplerType}
    );
    return nextID++;
}

bool mtd::ResourceManager::updateBufferData
(
    ResourceID id, vk::DeviceSize copySize, const void* srcData, vk::DeviceSize bufferOffset
)
{
    ResourceIterator r = resources.find(id);
    if(r == resources.end() || !std::holds_alternative<GpuBuffer>(r->second)) return false;

    std::get<GpuBuffer>(r->second).copyMemoryToBuffer(copySize, srcData, bufferOffset);
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
    ResourceConstIterator r = resources.find(id);
    if(r == resources.end() || !std::holds_alternative<Image>(r->second)) return false;

    std::get<Image>(r->second).transitionImageLayout(commandBuffer, newLayout, srcStage, dstStage);
    return true;
}

bool mtd::ResourceManager::deleteResource(ResourceID id)
{
    return resources.erase(id) > 0UL;
}

void mtd::ResourceManager::clearResources()
{
    resources.clear();
    nextID = 1U;
}

bool mtd::ResourceManager::updateDescriptorBufferInfo(ResourceID id, vk::DescriptorBufferInfo& info) const
{
    ResourceConstIterator r = resources.find(id);
    if(r == resources.end() || !std::holds_alternative<GpuBuffer>(r->second)) return false;

    std::get<GpuBuffer>(r->second).updateDescriptorInfo(info);
    return true;
}

bool mtd::ResourceManager::updateDescriptorImageInfo(ResourceID id, vk::DescriptorImageInfo& info) const
{
    ResourceConstIterator r = resources.find(id);
    if(r == resources.end() || !std::holds_alternative<Image>(r->second)) return false;

    std::get<Image>(r->second).updateDescriptorInfo(info);
    return true;
}
