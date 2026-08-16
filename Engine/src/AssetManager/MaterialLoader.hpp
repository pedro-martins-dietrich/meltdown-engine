#pragma once

#include "ResourceManager.hpp"

// Responsible for loading materials from files to the GPU
namespace mtd::MaterialLoader
{
    // Loads all scene textures from the disk to the GPU
    void loadTextures
    (
        ResourceManager& resourceManager,
        const std::vector<std::string>& texturePaths,
        std::vector<ResourceID>& textureIDs
    );

    // Loads all scene materials and material sets to the GPU
    void loadMaterials
    (
        ResourceManager& resourceManager,
        const std::vector<std::string>& materialFiles,
        const std::vector<std::vector<uint32_t>>& sets,
        ResourceID& materialBufferID,
        ResourceID& materialIndexingBufferID,
        ResourceID& materialSetBufferID
    );
}
