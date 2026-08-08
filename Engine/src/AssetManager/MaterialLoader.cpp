#include <pch.hpp>
#include "MaterialLoader.hpp"

#include "../Utils/EngineStructs.hpp"
#include "../Utils/Logger.hpp"
#include "../Utils/StringParser.hpp"

namespace mtd::MaterialLoader
{
    constexpr uint64_t MATERIAL_MAGIC = "MTD_MTRL"_u64;
    constexpr uint64_t MATERIAL_FILE_VERSION = 1UL;
    constexpr size_t MATERIAL_ALIGNMENT = 16UL;

    static bool loadFromFile(std::string_view filePath, std::vector<std::byte>& materialData);
}

void mtd::MaterialLoader::loadMaterials
(
    ResourceManager& resourceManager,
    const std::vector<std::string>& materialFiles,
    const std::vector<std::vector<uint32_t>>& sets,
    ResourceID& materialBufferID,
    ResourceID& materialIndexingBufferID,
    ResourceID& materialSetBufferID
)
{
    if(materialFiles.size() == 0UL)
    {
        materialBufferID = 0U;
        materialIndexingBufferID = 0U;
        materialSetBufferID = 0U;
        return;
    }

    std::vector<std::byte> materialData;
    std::vector<uint32_t> materialIndexing;
    std::vector<uint32_t> materialSetData;
    materialIndexing.reserve(materialFiles.size());

    uint32_t currentMaterialOffset = 0U;
    for(std::string_view materialPath: materialFiles)
    {
        if(!loadFromFile(materialPath, materialData)) continue;
        materialIndexing.push_back(currentMaterialOffset);
        currentMaterialOffset = static_cast<uint32_t>(materialData.size() >> 4);
    }

    for(const std::vector<uint32_t>& set: sets)
        materialSetData.insert(materialSetData.end(), set.begin(), set.end());
    if(materialSetData.empty())
        materialSetData.push_back(0U);

    materialBufferID = resourceManager.createBuffer
    (
        "MaterialBuffer", GpuBufferType::Storage, GpuMemoryUsage::GpuOnly, materialData.size(), materialData.data()
    );
    materialIndexingBufferID = resourceManager.createBuffer
    (
        "MaterialIndexingBuffer", GpuBufferType::Storage, GpuMemoryUsage::GpuOnly,
        sizeof(uint32_t) * materialIndexing.size(), materialIndexing.data()
    );
    materialSetBufferID = resourceManager.createBuffer
    (
        "MaterialSetBuffer", GpuBufferType::Storage, GpuMemoryUsage::GpuOnly,
        sizeof(uint32_t) * materialSetData.size(), materialSetData.data()
    );

    LOG_INFO("Loaded %d materials.", materialFiles.size());
}

bool mtd::MaterialLoader::loadFromFile(std::string_view filePath, std::vector<std::byte>& materialData)
{
    std::ifstream materialFile{filePath.data(), std::ios::binary | std::ios::ate};
    if(!materialFile)
    {
        LOG_WARNING("Failed to find material file \"%s\" for loading.", filePath.data());
        return false;
    }

    std::streamsize materialFileSize = materialFile.tellg();
    if(materialFileSize < static_cast<std::streamsize>(sizeof(AssetHeader)))
    {
        LOG_WARNING("Invalid header for material file \"%s\".", filePath.data());
        return false;
    }
    size_t materialSize = materialFileSize - sizeof(AssetHeader);

    AssetHeader materialHeader{};
    materialFile.seekg(0, std::ios::beg);
    materialFile.read(reinterpret_cast<char*>(&materialHeader), sizeof(AssetHeader));

    bool validMaterialFile = true;
    validMaterialFile &= (materialHeader.magic == MATERIAL_MAGIC);
    validMaterialFile &= (materialHeader.version == MATERIAL_FILE_VERSION);
    validMaterialFile &= (materialSize > 0UL);
    if(!validMaterialFile)
    {
        LOG_WARNING("Invalid material file \"%s\".", filePath.data());
        return false;
    }

    size_t oldMaterialLumpSize = materialData.size();
    size_t newMaterialLumpSize = oldMaterialLumpSize + materialSize;
    size_t padding = (MATERIAL_ALIGNMENT - (newMaterialLumpSize % MATERIAL_ALIGNMENT)) % MATERIAL_ALIGNMENT;

    materialData.resize(newMaterialLumpSize + padding);
    std::span<std::byte> loadTarget{materialData.data() + oldMaterialLumpSize, materialSize};

    materialFile.seekg(sizeof(AssetHeader), std::ios::beg);
    materialFile.read(reinterpret_cast<char*>(loadTarget.data()), loadTarget.size());
    materialFile.close();

    LOG_VERBOSE("Material loaded from \"%s\".", filePath.data());
    return true;
}
