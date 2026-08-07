#pragma once

#include "ResourceManager.hpp"

namespace mtd
{
    // Handles all materials used in the scene
    class MaterialManager
    {
        public:
            MaterialManager() = default;
            ~MaterialManager() = default;

            MaterialManager(const MaterialManager&) = delete;
            MaterialManager& operator=(const MaterialManager&) = delete;

            // Loads all scene materials and material sets to the GPU
            void loadMaterials
            (
                ResourceManager& resourceManager,
                const std::vector<std::string>& materialFiles,
                const std::vector<std::vector<uint32_t>>& sets
            );

        private:
            // GPU buffer ID for the material data
            ResourceID materialBufferID = 0U;
            // GPU buffer ID for the material indexing
            ResourceID materialIndexingBufferID = 0U;
            // GPU buffer ID for the material sets
            ResourceID materialSetBufferID = 0U;

            // Flag to indicate if there is any material loaded in the GPU
            bool anyMaterialLoaded = false;

            // Loads a .mtrl file and appends it to the material data
            bool loadFromFile(std::string_view filePath, std::vector<std::byte>& materialData);
    };
}
