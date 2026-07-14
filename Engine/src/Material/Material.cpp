#include <pch.hpp>
#include "Material.hpp"

#include "../Utils/Logger.hpp"

mtd::Material::Material(const MaterialInfo& materialInfo)
	: floatAttributeTypes{materialInfo.floatDataTypes}, textureTypes{materialInfo.textureTypes}
{
	uint32_t offset = 0U;
	floatAttributeOffsets.reserve(floatAttributeTypes.size());
	for(MaterialFloatDataType floatDataType: floatAttributeTypes)
	{
		floatAttributeOffsets[floatDataType] = offset;
		offset += mapFloatDataTypeDimension(floatDataType);
	}
	offset += offset % 4U;
	floatAttributes.resize(offset);
}

mtd::Material::Material(Material&& other) noexcept
	: floatAttributeTypes{other.floatAttributeTypes},
	textureTypes{other.textureTypes},
	floatAttributeOffsets{other.floatAttributeOffsets},
	texturePaths{other.texturePaths},
	floatAttributes{other.floatAttributes},
	textures{std::move(other.textures)}
{}

void mtd::Material::fetchTexturePaths(std::vector<std::string>& textureFilePaths) const
{
	for(const auto& [textureType, texturePath]: texturePaths)
		textureFilePaths.emplace_back(texturePath.c_str());
}

void mtd::Material::addFloatData(MaterialFloatDataType floatDataType, const float* data)
{
	if(floatAttributeOffsets.find(floatDataType) == floatAttributeOffsets.end()) return;

	for(uint32_t i = 0U; i < mapFloatDataTypeDimension(floatDataType); i++)
		floatAttributes[floatAttributeOffsets.at(floatDataType) + i] = data[i];
}

void mtd::Material::addTexturePath(MaterialTextureType textureType, std::string&& texturePath)
{
	texturePaths[textureType] = std::move(texturePath);
}

void mtd::Material::loadMaterial
(
	const Device& device,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t bindingOffset
)
{
	loadFloatData(device, descriptorSetHandler, bindingOffset);
	loadTextures(device, commandHandler, descriptorSetHandler, bindingOffset);
}

void mtd::Material::loadFloatData
(
	const Device& device, DescriptorSetHandler& descriptorSetHandler, uint32_t bindingOffset
) const
{
	if(floatAttributeTypes.empty()) return;

	uint32_t dataSize = sizeof(float) * floatAttributes.size();

	descriptorSetHandler.createDescriptorResources
	(
		device, dataSize, vk::BufferUsageFlagBits::eUniformBuffer, bindingOffset
	);
	descriptorSetHandler.updateDescriptorData(bindingOffset, floatAttributes.data(), dataSize);

	if(textureTypes.empty() && bindingOffset == 0U)
		descriptorSetHandler.writeDescriptorSet();
}

void mtd::Material::loadTextures
(
	const Device& device,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t bindingOffset
)
{
	for(MaterialTextureType textureType: textureTypes)
	{
		if(texturePaths.find(textureType) == texturePaths.end())
		{
			LOG_ERROR("No file path could be found to the material texture of type %d.", textureType);
			return;
		}

		const char* texturePath = texturePaths.at(textureType).c_str();
		uint32_t binding = bindingOffset + (floatAttributeTypes.empty() ? 0U : 1U);
		textures.emplace_back(device, texturePath, commandHandler, descriptorSetHandler, binding);
	}
}

uint32_t mtd::Material::mapFloatDataTypeDimension(MaterialFloatDataType floatDataType) const
{
	switch(floatDataType)
	{
		case MaterialFloatDataType::DiffuseColor:
			return 4U;
		case MaterialFloatDataType::Emission:
			return 3U;
		case MaterialFloatDataType::IndexOfRefraction:
			return 1U;
		case MaterialFloatDataType::Roughness:
			return 1U;
		case MaterialFloatDataType::Metallic:
			return 1U;
		default:
			assert(false && "No size has been specified for the requested float data type.");
	}
	return 0U;
}
