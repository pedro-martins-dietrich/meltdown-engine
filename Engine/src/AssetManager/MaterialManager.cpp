#include <pch.hpp>
#include "MaterialManager.hpp"

#include "AssetLoad/MaterialLoader.hpp"
#include "../Utils/Logger.hpp"

mtd::MaterialManager::MaterialManager(const Device& device)
    : materialBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
    materialIndexingBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
    materialSetBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
    commandHandler{device}
{}

void mtd::MaterialManager::loadMaterials
(
    const std::vector<std::string>& materialFiles, const std::vector<std::vector<uint32_t>>& sets
)
{
    if(materialFiles.size() == 0)
    {
        anyMaterialLoaded = false;
        return;
    }

    std::vector<std::byte> materialData;
    std::vector<uint32_t> materialIndexing;
    std::vector<uint32_t> materialSetData;
    materialIndexing.reserve(materialFiles.size());

    uint32_t currentMaterialOffset = 0U;
    for(std::string_view materialPath: materialFiles)
    {
        materialIndexing.push_back(currentMaterialOffset);
        MaterialLoader::loadMaterial(materialPath, materialData);
        currentMaterialOffset = static_cast<uint32_t>(materialData.size() >> 4);
    }

    for(const std::vector<uint32_t>& set: sets)
        materialSetData.insert(materialSetData.end(), set.begin(), set.end());
    if(materialSetData.empty())
        materialSetData.push_back(0U);

    loadToGpu(materialData, materialIndexing, materialSetData);
    anyMaterialLoaded = true;

    LOG_INFO("Loaded %d materials.", materialFiles.size());
}

void mtd::MaterialManager::createMaterialDescriptor
(
    DescriptorSetHandler& descriptorSetHandler,
    uint32_t materialDataBinding,
    uint32_t materialIndexingBinding,
    uint32_t materialSetBinding
) const
{
    if(!anyMaterialLoaded)
    {
        LOG_VERBOSE("No materials loaded. Material descriptors will not be used.");
        return;
    }

    descriptorSetHandler.assignExternalResourcesToDescriptor(0U, materialDataBinding, materialBuffer);
    descriptorSetHandler.assignExternalResourcesToDescriptor(0U, materialIndexingBinding, materialIndexingBuffer);
    descriptorSetHandler.assignExternalResourcesToDescriptor(0U, materialSetBinding, materialSetBuffer);
}

void mtd::MaterialManager::loadToGpu
(
    const std::vector<std::byte>& materialData,
    const std::vector<uint32_t>& materialIndexing,
    const std::vector<uint32_t>& materialSetData
)
{
    if(!gpuBuffersCreated)
    {
        materialBuffer.createDeviceLocal(commandHandler, materialData.size(), materialData.data());
        materialIndexingBuffer.createDeviceLocal
        (
            commandHandler, sizeof(uint32_t) * materialIndexing.size(), materialIndexing.data()
        );
        materialSetBuffer.createDeviceLocal
        (
            commandHandler, sizeof(uint32_t) * materialSetData.size(), materialSetData.data()
        );

        gpuBuffersCreated = true;
        return;
    }

    materialBuffer.resizeBuffer(commandHandler, materialData.size());
    materialBuffer.copyMemoryToBuffer(materialData.size(), materialData.data());
    materialIndexingBuffer.resizeBuffer(commandHandler, sizeof(uint32_t) * materialIndexing.size());
    materialIndexingBuffer.copyMemoryToBuffer(sizeof(uint32_t) * materialIndexing.size(), materialIndexing.data());
    materialSetBuffer.resizeBuffer(commandHandler, sizeof(uint32_t) * materialSetData.size());
    materialSetBuffer.copyMemoryToBuffer(sizeof(uint32_t) * materialSetData.size(), materialSetData.data());
}
