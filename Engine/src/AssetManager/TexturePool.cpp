#include <pch.hpp>
#include "TexturePool.hpp"

#include "../Utils/FileHandler.hpp"
#include "../Utils/Logger.hpp"

static constexpr mtd::UIntVec2 MISSING_TEXTURE_DIMENSIONS{16U, 16U};
static constexpr std::array<uint32_t, 256> MISSING_TEXTURE = []() constexpr
{
	std::array<uint32_t, 256> texture{};
	for(size_t i = 0UL; i < 256UL; i++)
		texture[i] = (i % 2 ^ (i >> 4) % 2) ? 0xFF000000 : 0xFFFF00FF;
	return texture;
}();

void mtd::TexturePool::loadTextures(ResourceManager& resourceManager, const std::vector<std::string>& texturePaths)
{
    textureIDs.clear();
    textureIDs.reserve(texturePaths.size());

    missingTextureID = resourceManager.loadImage
    (
        "MissingTexture",
        MISSING_TEXTURE_DIMENSIONS,
        MISSING_TEXTURE.data(),
        MISSING_TEXTURE.size() * sizeof(uint32_t),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled
    );

    for(size_t i = 0U; i < texturePaths.size(); i++)
    {
        UIntVec2 dimensions{0U, 0U};
        uint32_t channels = 0U;
        void* pImageData = FileHandler::readImage(texturePaths[i], dimensions, channels);
        if(!pImageData)
        {
            textureIDs.emplace_back(missingTextureID);
            continue;
        }

        vk::Format imageFormat = vk::Format::eUndefined;
        switch(channels)
        {
            case 1U:
                imageFormat = vk::Format::eR8Unorm;
                break;
            case 2U:
                imageFormat = vk::Format::eR8G8Unorm;
                break;
            case 4U:
                imageFormat = vk::Format::eR8G8B8A8Unorm;
                break;
            default:
                LOG_ERROR("Unsupported image format: %d channels.", channels);
        }

        textureIDs.emplace_back(resourceManager.loadImage
        (
            "", dimensions, pImageData, static_cast<size_t>(dimensions.x * dimensions.y * channels),
            imageFormat, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled
        ));

        free(pImageData);
    }
}

void mtd::TexturePool::fetchTextureDescriptorInfos
(
    const ResourceManager& resourceManager, std::vector<vk::DescriptorImageInfo>& descriptorImageInfos
) const
{
    descriptorImageInfos.resize(MAX_TEXTURE_POOL_SIZE);
    size_t index = 0UL;
	for(; index < textureIDs.size(); index++)
        resourceManager.fetchDescriptorImageInfo(textureIDs[index], descriptorImageInfos[index]);
    for(; index < MAX_TEXTURE_POOL_SIZE; index++)
        resourceManager.fetchDescriptorImageInfo(missingTextureID, descriptorImageInfos[index]);
}
