#include <pch.hpp>
#include "MaterialManager.hpp"

#include "Import/MaterialImporter.hpp"
#include "../Utils/Logger.hpp"

mtd::MaterialManager::MaterialManager(const Device& device)
    : materialBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
    materialIndexingBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
    commandHandler{device}
{}

void mtd::MaterialManager::loadMaterials(const std::vector<std::string>& materialFiles)
{
    if(materialFiles.size() == 0)
    {
        anyMaterialLoaded = false;
        return;
    }

    std::vector<std::byte> materialData;
    std::vector<uint32_t> materialIndexing;
    materialIndexing.reserve(materialFiles.size());

    uint32_t currentMaterialOffset = 0U;
    for(std::string_view materialPath: materialFiles)
    {
        materialIndexing.push_back(currentMaterialOffset);
        MaterialImporter::loadMaterial(materialPath, materialData);
        currentMaterialOffset = static_cast<uint32_t>(materialData.size() >> 4);
    }

    loadToGpu(materialData, materialIndexing);
    anyMaterialLoaded = true;

    LOG_INFO("Loaded %d materials.", materialFiles.size());
}

void mtd::MaterialManager::createMaterialDescriptor
(
    DescriptorSetHandler& descriptorSetHandler,
    uint32_t materialIndexingBinding,
    uint32_t materialDataBinding
) const
{
    if(!anyMaterialLoaded)
    {
        LOG_VERBOSE("No materials loaded. Material descriptors will not be used.");
        return;
    }

    descriptorSetHandler.assignExternalResourcesToDescriptor(0U, materialIndexingBinding, materialIndexingBuffer);
    descriptorSetHandler.assignExternalResourcesToDescriptor(0U, materialDataBinding, materialBuffer);
}

void mtd::MaterialManager::loadToGpu(const std::vector<std::byte>& materialData, const std::vector<uint32_t>& materialIndexing)
{
    if(!gpuBuffersCreated)
    {
        materialBuffer.createDeviceLocal(commandHandler, materialData.size(), materialData.data());
        materialIndexingBuffer.createDeviceLocal
        (
            commandHandler, sizeof(uint32_t) * materialIndexing.size(), materialIndexing.data()
        );

        gpuBuffersCreated = true;
        return;
    }

    materialBuffer.resizeBuffer(commandHandler, materialData.size());
    materialBuffer.copyMemoryToBuffer(materialData.size(), materialData.data());
    materialIndexingBuffer.resizeBuffer(commandHandler, sizeof(uint32_t) * materialIndexing.size());
    materialIndexingBuffer.copyMemoryToBuffer(sizeof(uint32_t) * materialIndexing.size(), materialIndexing.data());
}
