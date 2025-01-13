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
{
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
	uint32_t swappableSetIndex
)
{
	loadFloatData(device, descriptorSetHandler, swappableSetIndex);
	loadTextures(device, commandHandler, descriptorSetHandler, swappableSetIndex);
}

// Loads all float attributes to the GPU
void mtd::Material::loadFloatData
(
	const Device& device,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t swappableSetIndex
) const
{
	if(floatAttributeTypes.empty()) return;

	uint32_t dataSize = sizeof(float) * floatAttributes.size();

	descriptorSetHandler.createDescriptorResources
	(
		device, dataSize, vk::BufferUsageFlagBits::eUniformBuffer, swappableSetIndex, 0
	);
	descriptorSetHandler.updateDescriptorData(swappableSetIndex, 0, floatAttributes.data(), dataSize);

	if(textureTypes.empty())
		descriptorSetHandler.writeDescriptorSet(swappableSetIndex);
}

// Loads all material textures
void mtd::Material::loadTextures
(
	const Device& device,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t swappableSetIndex
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
		uint32_t binding = floatAttributeTypes.empty() ? 0 : 1;
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
