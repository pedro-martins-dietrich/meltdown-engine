#include <pch.hpp>
#include "MaterialIO.hpp"

#include "AssetFileHeaders.hpp"
#include "../Utils/FileHandler.hpp"
#include "../Utils/Logger.hpp"
#include "../Utils/StringParser.hpp"

namespace mtd
{
	constexpr uint64_t MATERIAL_MAGIC = "MTD_MTRL"_u64;
	constexpr uint32_t MATERIAL_FILE_VERSION = 1U;
}

bool mtd::MaterialIO::saveMaterial(std::string_view fileName, const Material& material)
{
	std::string materialPath{MTD_RESOURCES_PATH};
	materialPath.append("materials/");
	materialPath.append(fileName);
	materialPath.append(".mtrl");

	std::ofstream materialFile{materialPath, std::ios::binary};
	if(!materialFile)
	{
		LOG_WARNING("Failed to save material to \"%s\".", materialPath.c_str());
		return false;
	}

	MaterialHeader header{};
	header.magic = MATERIAL_MAGIC;
	header.version = MATERIAL_FILE_VERSION;

	const std::vector<MaterialFloatDataType>& floatAttributeTypes = material.getFloatAttributeTypes();
	const std::vector<MaterialTextureType>& textureTypes = material.getTextureTypes();

	header.floatDataTypeCount = floatAttributeTypes.size();
	header.textureCount = textureTypes.size();
	header.floatDataCount = material.getFloatAttributesSize();

	size_t floatAttributeTypesSize = floatAttributeTypes.size() * sizeof(MaterialFloatDataType);
	size_t textureTypesSize = textureTypes.size() * sizeof(MaterialTextureType);
	size_t floatDataSize = material.getFloatAttributesSize() * sizeof(float);

	size_t currentOffset = sizeof(MaterialHeader);
	header.floatDataTypesOffset = static_cast<uint32_t>(currentOffset);
	currentOffset += floatAttributeTypesSize;
	header.textureTypesOffset = static_cast<uint32_t>(currentOffset);
	currentOffset += textureTypesSize;
	header.floatDataOffset = static_cast<uint32_t>(currentOffset);
	currentOffset += floatDataSize;
	header.texturePathsOffset = static_cast<uint32_t>(currentOffset);

	header.flags = 0U;

	materialFile.write(reinterpret_cast<char*>(&header), sizeof(MaterialHeader));
	materialFile.write(reinterpret_cast<const char*>(floatAttributeTypes.data()), floatAttributeTypesSize);
	materialFile.write(reinterpret_cast<const char*>(textureTypes.data()), textureTypesSize);
	materialFile.write(reinterpret_cast<const char*>(material.getFloatAttributesData()), floatDataSize);

	for(MaterialTextureType textureType: textureTypes)
	{
		if(!FileHandler::writeString(materialFile, material.getTexturePath(textureType)))
			return false;
	}

	materialFile.close();

	LOG_INFO("Material saved to \"%s\".", materialPath.c_str());

	return true;
}

bool mtd::MaterialIO::loadMaterial(std::string_view fileName, Material& material)
{
	std::string materialPath{MTD_RESOURCES_PATH};
	materialPath.append("materials/");
	materialPath.append(fileName);
	materialPath.append(".mtrl");

	std::ifstream materialFile{materialPath, std::ios::binary | std::ios::ate};
	if(!materialFile)
	{
		LOG_WARNING("Failed to find material file \"%s\" for loading.", materialPath.c_str());
		return false;
	}

	std::streamsize materialFileSize = materialFile.tellg();
	if(materialFileSize < static_cast<std::streamsize>(sizeof(MaterialHeader)))
	{
		LOG_WARNING("Invalid header for material file \"%s\".", materialPath.c_str());
		return false;
	}

	MaterialHeader header{};
	materialFile.seekg(0, std::ios::beg);
	materialFile.read(reinterpret_cast<char*>(&header), sizeof(MaterialHeader));

	bool validMaterialFile = true;
	validMaterialFile &= header.magic == MATERIAL_MAGIC;
	validMaterialFile &= header.version == MATERIAL_FILE_VERSION;

	size_t floatDataTypesSize = header.floatDataTypeCount * sizeof(MaterialFloatDataType);
	size_t textureTypesSize = header.textureCount * sizeof(MaterialTextureType);
	size_t floatDataSize = header.floatDataCount * sizeof(float);

	validMaterialFile &= header.floatDataTypesOffset <= static_cast<uint32_t>(materialFileSize);
	validMaterialFile &= header.textureTypesOffset <= static_cast<uint32_t>(materialFileSize);
	validMaterialFile &= header.floatDataOffset <= static_cast<uint32_t>(materialFileSize);
	validMaterialFile &= header.texturePathsOffset <= static_cast<uint32_t>(materialFileSize);
	validMaterialFile &= (header.floatDataTypesOffset + floatDataTypesSize) <= static_cast<size_t>(materialFileSize);
	validMaterialFile &= (header.textureTypesOffset + textureTypesSize) <= static_cast<size_t>(materialFileSize);
	validMaterialFile &= (header.floatDataOffset + floatDataSize) <= static_cast<size_t>(materialFileSize);

	if(!validMaterialFile)
	{
		LOG_WARNING("Invalid material file \"%s\".", materialPath.c_str());
		return false;
	}

	std::vector<MaterialFloatDataType>& floatAttributeTypes = material.getFloatAttributeTypes();
	std::vector<MaterialTextureType>& textureTypes = material.getTextureTypes();
	std::vector<float>& floatAttributes = material.getFloatAttributes();
	floatAttributeTypes.resize(header.floatDataTypeCount);
	textureTypes.resize(header.textureCount);
	floatAttributes.resize(header.floatDataCount);

	materialFile.seekg(header.floatDataTypesOffset, std::ios::beg);
	materialFile.read(reinterpret_cast<char*>(floatAttributeTypes.data()), floatDataTypesSize);
	materialFile.seekg(header.textureTypesOffset, std::ios::beg);
	materialFile.read(reinterpret_cast<char*>(textureTypes.data()), textureTypesSize);
	materialFile.seekg(header.floatDataOffset, std::ios::beg);
	materialFile.read(reinterpret_cast<char*>(floatAttributes.data()), floatDataSize);

	materialFile.seekg(header.texturePathsOffset, std::ios::beg);
	for(MaterialTextureType textureType: textureTypes)
	{
		std::string texturePath;
		if(!FileHandler::readString(materialFile, texturePath))
		{
			LOG_WARNING("Failed to properly read material file \"%s\".", materialPath.c_str());
			return false;
		}
		material.addTexturePath(textureType, std::move(texturePath));
	}

	materialFile.close();

	LOG_INFO("Material loaded from \"%s\".", materialPath.c_str());

	return true;
}
