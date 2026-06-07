#pragma once

#include "../../Utils/EngineStructs.hpp"

// Handles all logic for loading a mesh from a .mesh file
namespace mtd::MeshLoader
{
    // Mesh asset file header
    struct MeshHeader : AssetHeader
    {
        uint32_t vertexStride;
    };

    // Loads a .mesh file and appends it to the mesh data
    bool loadMesh
    (
        const std::string_view filePath,
        std::vector<std::byte>& vertexData,
        std::vector<uint32_t>& indexData,
        std::vector<MeshData>& meshes
    );
}
