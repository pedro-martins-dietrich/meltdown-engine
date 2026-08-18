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

mtd::ResourceManager::ResourceManager(const Device& mtdDevice, UIntVec2 windowResolution)
    : mtdDevice{mtdDevice}, windowResolution{windowResolution}, commandHandler{mtdDevice}
{
    configureEventCallbacks();
}

mtd::ResourceID mtd::ResourceManager::getResourceID(std::string_view resourceName) const
{
    ResourceIdConstIterator nameIdIterator = nameIdMap.find(resourceName.data());
    if(nameIdIterator == nameIdMap.cend()) return 0U;

    return nameIdIterator->second;
}

vk::Buffer mtd::ResourceManager::getVulkanBuffer(ResourceID bufferID) const
{
    if(bufferID == 0U) return nullptr;
    BufferConstIterator bufferIterator = buffers.find(bufferID);
    if(bufferIterator == buffers.cend()) return nullptr;

    return bufferIterator->second.getBuffer();
}

uint64_t mtd::ResourceManager::getBufferSize(ResourceID bufferID) const
{
    if(bufferID == 0U) return 0UL;
    BufferConstIterator bufferIterator = buffers.find(bufferID);
    if(bufferIterator == buffers.cend()) return 0UL;

    return bufferIterator->second.getSize();
}

mtd::ResourceID mtd::ResourceManager::createBuffer
(
    std::string resourceName, GpuBufferType type, GpuMemoryUsage memoryUsage, uint64_t bufferSize, const void* pData
)
{
    if(memoryUsage == GpuMemoryUsage::Auto)
        memoryUsage = EnumMapping::defaultMemoryUsage(type);
    vk::BufferUsageFlags bufferUsage = EnumMapping::getBufferUsage(type);
    vk::MemoryPropertyFlags memoryProperties = EnumMapping::getMemoryProperties(memoryUsage);

    if(bufferSize == 0UL)
        buffers.emplace(nextID, GpuBuffer{mtdDevice, bufferUsage, memoryProperties});
    else
    {
        buffers.emplace(nextID, GpuBuffer{mtdDevice, bufferSize, bufferUsage, memoryProperties});
        if(pData)
            buffers.at(nextID).copyMemoryToBuffer(bufferSize, pData);
    }

    if(resourceName.length() != 0UL)
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
    vk::ImageAspectFlags aspects,
    vk::ImageViewType viewType,
    vk::MemoryPropertyFlags memoryProperties
)
{
    if(windowResolutionRatio.x > 0.0f)
        imageDimensions.x = static_cast<uint32_t>(windowResolutionRatio.x * windowResolution.x);
    if(windowResolutionRatio.y > 0.0f)
        imageDimensions.y = static_cast<uint32_t>(windowResolutionRatio.y * windowResolution.y);

    images.emplace
    (
        nextID,
        Image
        {
            mtdDevice,
            imageDimensions,
            imageFormat,
            tiling,
            usage,
            memoryProperties,
            aspects,
            viewType,
            samplerType,
            windowResolutionRatio
        }
    );

    if(resourceName.length() != 0UL)
        nameIdMap.emplace(std::move(resourceName), nextID);

    return nextID++;
}

mtd::ResourceID mtd::ResourceManager::loadImage
(
    std::string resourceName,
    UIntVec2 imageDimensions,
    const void* pData,
    uint64_t dataSize,
    vk::Format imageFormat,
    vk::ImageUsageFlags usage,
    SamplerType samplerType,
    vk::ImageTiling tiling,
    vk::ImageAspectFlags aspects,
    vk::ImageViewType viewType,
    vk::MemoryPropertyFlags memoryProperties
)
{
    if(!pData || dataSize == 0UL || imageDimensions.x == 0U || imageDimensions.y == 0U)
    {
        LOG_ERROR("Cannot create GPU image resource from an invalid image.");
        return 0U;
    }

    images.emplace
    (
        nextID,
        Image{mtdDevice, imageDimensions, imageFormat, tiling, usage, memoryProperties, aspects, viewType, samplerType}
    );
    GpuBuffer stagingBuffer
    {
        mtdDevice, dataSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible
    };
    stagingBuffer.copyMemoryToBuffer(dataSize, pData);
    images.at(nextID).copyBufferToImage(commandHandler, stagingBuffer.getBuffer());

    if(resourceName.length() != 0UL)
        nameIdMap.emplace(std::move(resourceName), nextID);

    return nextID++;
}

bool mtd::ResourceManager::updateBufferData
(
    ResourceID id, uint64_t copySize, const void* srcData, uint64_t bufferOffset
)
{
    if(id == 0U) return false;
    BufferIterator bufferIterator = buffers.find(id);
    if(bufferIterator == buffers.cend()) return false;

    bufferIterator->second.copyMemoryToBuffer(copySize, srcData, bufferOffset);
    return true;
}

bool mtd::ResourceManager::resizeBuffer(ResourceID id, uint64_t newSize)
{
    if(id == 0U) return false;
    BufferIterator bufferIterator = buffers.find(id);
    if(bufferIterator == buffers.cend()) return false;

    bufferIterator->second.resizeBuffer(commandHandler, newSize);
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
    if(id == 0U) return false;
    ImageConstIterator imageIterator = images.find(id);
    if(imageIterator == images.cend()) return false;

    imageIterator->second.transitionImageLayout(commandBuffer, newLayout, srcStage, dstStage);
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
    nameIdMap.clear();
}

bool mtd::ResourceManager::fetchDescriptorBufferInfo(ResourceID id, vk::DescriptorBufferInfo& info) const
{
    if(id == 0U) return false;
    BufferConstIterator bufferIterator = buffers.find(id);
    if(bufferIterator == buffers.cend()) return false;

    bufferIterator->second.updateDescriptorInfo(info);
    return true;
}

bool mtd::ResourceManager::fetchDescriptorImageInfo(ResourceID id, vk::DescriptorImageInfo& info) const
{
    if(id == 0U) return false;
    ImageConstIterator imageIterator = images.find(id);
    if(imageIterator == images.cend()) return false;

    imageIterator->second.updateDescriptorInfo(info);
    return true;
}

bool mtd::ResourceManager::fetchDescriptorImageInfos
(
    const std::vector<ResourceID>& ids, std::vector<vk::DescriptorImageInfo>& infos
) const
{
    uint32_t fails = 0U;
    infos.resize(ids.size());

    for(size_t i = 0; i < ids.size(); i++)
    {
        if(ids[i] == 0U) fails++;
        ImageConstIterator imageIterator = images.find(ids[i]);
        if(imageIterator == images.cend()) fails++;

        imageIterator->second.updateDescriptorInfo(infos[i]);
    }
    if(fails != 0U)
    {
        LOG_ERROR("Failed to fetch the descriptor image infos from %d out of %d images.", fails, ids.size());
        return false;
    }

    return true;
}

void mtd::ResourceManager::configureEventCallbacks()
{
    updateGpuBufferHandle = EventManager::addCallback([this](const UpdateGpuBufferEvent& event)
	{
        ResourceIdConstIterator nameIdIter = nameIdMap.find(event.name);
        if(nameIdIter == nameIdMap.cend()) return;

        updateBufferData(nameIdIter->second, event.copySize, event.pData, event.offset);
	});
}
