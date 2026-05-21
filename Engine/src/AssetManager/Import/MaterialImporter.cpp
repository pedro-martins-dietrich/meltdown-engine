#include <pch.hpp>
#include "MaterialImporter.hpp"

#include "../../Utils/EngineStructs.hpp"
#include "../../Utils/Logger.hpp"
#include "../../Utils/StringParser.hpp"

namespace mtd
{
    constexpr uint64_t MATERIAL_MAGIC = "MTD_MTRL"_u64;
    constexpr uint32_t MATERIAL_FILE_VERSION = 1U;
    constexpr size_t MATERIAL_ALIGNMENT = 16;
}

bool mtd::MaterialImporter::loadMaterial(std::string_view filePath, std::vector<std::byte>& materialData)
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
    validMaterialFile &= (materialSize > 0);
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
