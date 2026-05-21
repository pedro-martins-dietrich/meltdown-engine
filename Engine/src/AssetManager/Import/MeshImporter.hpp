#pragma once

#include "../../Utils/EngineStructs.hpp"

// Handles the logic for loading a mesh from a .mesh file
namespace mtd::MeshImporter
{
    // Loads a .mesh file and appends it to the mesh data
    bool loadMesh
    (
        const std::string_view filePath,
        std::vector<std::byte>& vertexData,
        std::vector<uint32_t>& indexData,
        std::vector<SubmeshData>& submeshes
    );
}
