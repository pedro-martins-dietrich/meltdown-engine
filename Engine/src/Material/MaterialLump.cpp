#include <pch.hpp>
#include "MaterialLump.hpp"

mtd::MaterialLump::MaterialLump(const Device& mtdDevice, const MaterialInfo& materialInfo)
	: mtdDevice{mtdDevice}, materialInfo{materialInfo},
	floatDataBuffer{mtdDevice, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal}
{}

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

void mtd::MaterialLump::assignFloatDataBufferToDescriptor
(
	DescriptorSetHandler& descriptorSetHandler, uint32_t bindingIndex
) const
{
	descriptorSetHandler.assignExternalResourcesToDescriptor(bindingIndex, floatDataBuffer);
}

void mtd::MaterialLump::assignTexturesToDescriptor
(
	DescriptorSetHandler& descriptorSetHandler, uint32_t bindingIndex
) const
{
	std::vector<vk::DescriptorImageInfo> descriptorImageInfos;
	for(const Texture& texture: textures)
	{
		vk::DescriptorImageInfo descriptorInfo;
		texture.updateDescriptorImageInfo(descriptorInfo);
		descriptorImageInfos.emplace_back(descriptorInfo);
	}

	descriptorSetHandler.createImagesDescriptorResources(bindingIndex, descriptorImageInfos);
}

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
