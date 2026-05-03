#pragma once

#include "../Utils/EngineStructs.hpp"
#include "../Vulkan/Image/Texture.hpp"

namespace mtd
{
    static constexpr uint32_t MAX_TEXTURE_POOL_SIZE = 1024U;

    // Handles all textures used by the current scene
    class TexturePool
    {
        public:
            TexturePool(const Device& mtdDevice);
            ~TexturePool() = default;

            TexturePool(const TexturePool&) = delete;
            TexturePool& operator=(const TexturePool&) = delete;

            // Loads all scene textures from the disk to the GPU
            void loadTextures(const std::vector<TextureInfo>& texturesInfo);

            // Creates the descriptor for the scene textures
            void createTextureListDescriptor(DescriptorSetHandler& descriptorSetHandler, uint32_t binding) const;

        private:
            // Command handler for texture loading
            CommandHandler commandHandler;

            // Texture pool
            std::vector<Texture> textures;
            // Missing texture
            Texture missingTexture;

            // Device reference
            const Device& mtdDevice;
    };
}
