#include <pch.hpp>
#include "Material.hpp"

#include "../Utils/Logger.hpp"

mtd::Material::Material(const MaterialInfo& materialInfo)
	: floatAttributeTypes{materialInfo.floatDataTypes}, textureTypes{materialInfo.textureTypes}
{
	uint32_t offset = 0;
	floatAttributeOffsets.reserve(floatAttributeTypes.size());
	for(MaterialFloatDataType floatDataType: floatAttributeTypes)
	{
		floatAttributeOffsets[floatDataType] = offset;
		offset += mapFloatDataTypeDimension(floatDataType);
	}
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

// Fetches the texture paths for the material
void mtd::Material::fetchTexturePaths(std::vector<std::string>& textureFilePaths) const
{
	for(const auto& [textureType, texturePath]: texturePaths)
		textureFilePaths.emplace_back(texturePath.c_str());
}

// Inserts a float attribute data
void mtd::Material::addFloatData(MaterialFloatDataType floatDataType, const float* data)
{
	if(floatAttributeOffsets.find(floatDataType) == floatAttributeOffsets.end()) return;

	for(uint32_t i = 0; i < mapFloatDataTypeDimension(floatDataType); i++)
		floatAttributes[floatAttributeOffsets.at(floatDataType) + i] = data[i];
}

// Adds a texture path to load the material
void mtd::Material::addTexturePath(MaterialTextureType textureType, std::string&& texturePath)
{
	texturePaths[textureType] = std::move(texturePath);
}

// Loads all float attributes and textures to the GPU
void mtd::Material::loadMaterial
(
	const Device& device,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t swappableSetIndex,
	uint32_t bindingOffset
)
{
	loadFloatData(device, descriptorSetHandler, swappableSetIndex, bindingOffset);
	loadTextures(device, commandHandler, descriptorSetHandler, swappableSetIndex, bindingOffset);
}

// Loads all float attributes to the GPU
void mtd::Material::loadFloatData
(
	const Device& device,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t swappableSetIndex,
	uint32_t bindingOffset
) const
{
	if(floatAttributeTypes.empty()) return;

	uint32_t dataSize = sizeof(float) * floatAttributes.size();

	descriptorSetHandler.createDescriptorResources
	(
		device, dataSize, vk::BufferUsageFlagBits::eUniformBuffer, swappableSetIndex, bindingOffset
	);
	descriptorSetHandler.updateDescriptorData(swappableSetIndex, bindingOffset, floatAttributes.data(), dataSize);

	if(textureTypes.empty() && bindingOffset == 0)
		descriptorSetHandler.writeDescriptorSet(swappableSetIndex);
}

// Loads all material textures
void mtd::Material::loadTextures
(
	const Device& device,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t swappableSetIndex,
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
		uint32_t binding = bindingOffset + (floatAttributeTypes.empty() ? 0 : 1);
		textures.emplace_back(device, texturePath, commandHandler, descriptorSetHandler, swappableSetIndex, binding);
	}
}

// Finds the dimension (amount of floats) of each material float data type
uint32_t mtd::Material::mapFloatDataTypeDimension(MaterialFloatDataType floatDataType) const
{
	switch(floatDataType)
	{
		case MaterialFloatDataType::DiffuseColor:
			return 4;
		default:
			assert(false && "No size has been specified for the requested float data type.");
	}
	return 0;
}
