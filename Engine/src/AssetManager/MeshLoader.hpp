#pragma once

#include "ResourceManager.hpp"

// Responsible for loading mesh data from files to the GPU
namespace mtd::MeshLoader
{
    // Loads all scene mesh data to the GPU
    void loadMeshes
    (
        ResourceManager& resourceManager,
        const std::vector<std::string>& meshFiles,
        std::vector<MeshData>& meshes,
        ResourceID& vertexBufferID,
        ResourceID& indexBufferID,
        ResourceID& submeshBufferID
    );
}
