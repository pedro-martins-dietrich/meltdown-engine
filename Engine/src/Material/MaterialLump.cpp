#include <pch.hpp>
#include "MaterialLump.hpp"

mtd::MaterialLump::MaterialLump(const Device& mtdDevice, const MaterialInfo& materialInfo)
	: mtdDevice{mtdDevice},
	materialInfo{materialInfo},
	floatDataBuffer{mtdDevice, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
	materialCount{0U}
{}

// Loads material float data and textures
void mtd::MaterialLump::addMaterial(const float* data, size_t size, const std::vector<std::string>& texturePaths)
{
	floatMaterialAttributes.insert(floatMaterialAttributes.cend(), data, data + size);

	assert
	(
		materialInfo.textureTypes.size() == texturePaths.size() &&
		"The number of textures should match the material type."
	);

	for(const std::string& texturePath: texturePaths)
		textures.emplace_back(mtdDevice, texturePath.c_str());

	materialCount++;
}

// Assigns the material float data buffer as a descriptor
void mtd::MaterialLump::assignFloatDataBufferToDescriptor
(
	DescriptorSetHandler& descriptorSetHandler, uint32_t swappableSetIndex, uint32_t bindingIndex
) const
{
	descriptorSetHandler.assignExternalResourcesToDescriptor(swappableSetIndex, bindingIndex, floatDataBuffer);
}

// Assigns the material textures to a descriptor
void mtd::MaterialLump::assignTexturesToDescriptor
(
	DescriptorSetHandler& descriptorSetHandler, uint32_t swappableSetIndex, uint32_t bindingIndex
) const
{
	std::vector<vk::DescriptorImageInfo> descriptorImageInfos;
	for(const Texture& texture: textures)
		descriptorImageInfos.emplace_back(texture.getDescriptorImageInfo());

	descriptorSetHandler.createImagesDescriptorResources(swappableSetIndex, bindingIndex, descriptorImageInfos);
}

// Loads all materials to the GPU VRAM
void mtd::MaterialLump::loadMaterialsToGPU(const CommandHandler& commandHandler)
{
	floatDataBuffer.createDeviceLocal
	(
		commandHandler, sizeof(float) * floatMaterialAttributes.size(), floatMaterialAttributes.data()
	);
	floatMaterialAttributes.clear();

	for(Texture& texture: textures)
		texture.loadToGpu(mtdDevice, commandHandler);
}
