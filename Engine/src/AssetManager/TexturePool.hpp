#pragma once

#include "ResourceManager.hpp"

namespace mtd
{
    static constexpr uint32_t MAX_TEXTURE_POOL_SIZE = 1024U;

    // Handles all textures used by the current scene
    class TexturePool
    {
        public:
            TexturePool() = default;
            ~TexturePool() = default;

            TexturePool(const TexturePool&) = delete;
            TexturePool& operator=(const TexturePool&) = delete;

            // Loads all scene textures from the disk to the GPU
            void loadTextures(ResourceManager& resourceManager, const std::vector<std::string>& texturePaths);

            // Creates the descriptor for the scene textures
            void fetchTextureDescriptorInfos
            (
                const ResourceManager& resourceManager, std::vector<vk::DescriptorImageInfo>& descriptorImageInfos
            ) const;

        private:
            // Texture IDs
            std::vector<ResourceID> textureIDs;
            // Missing texture ID
            ResourceID missingTextureID = 0U;
    };
}
