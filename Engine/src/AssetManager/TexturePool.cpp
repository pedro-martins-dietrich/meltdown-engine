#include <pch.hpp>
#include "TexturePool.hpp"

mtd::TexturePool::TexturePool(const Device& mtdDevice)
    : mtdDevice{mtdDevice}, commandHandler{mtdDevice}, missingTexture{mtdDevice, commandHandler}
{}

void mtd::TexturePool::loadTextures(const std::vector<TextureInfo>& texturesInfo)
{
    textures.clear();
    textures.reserve(texturesInfo.size());
    for(const TextureInfo& info: texturesInfo)
        textures.emplace_back(mtdDevice, info.texturePath, commandHandler);
}

void mtd::TexturePool::createTextureListDescriptor(DescriptorSetHandler& descriptorSetHandler, uint32_t binding) const
{
    std::vector<vk::DescriptorImageInfo> descriptorImageInfos(MAX_TEXTURE_POOL_SIZE);
    size_t index = 0UL;
	for(; index < textures.size(); index++)
		textures[index].updateDescriptorImageInfo(descriptorImageInfos[index]);
    for(; index < MAX_TEXTURE_POOL_SIZE; index++)
        missingTexture.updateDescriptorImageInfo(descriptorImageInfos[index]);

	descriptorSetHandler.createImagesDescriptorResources(binding, descriptorImageInfos);
}
