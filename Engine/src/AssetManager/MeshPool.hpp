#pragma once

#include "ResourceManager.hpp"
#include "../Vulkan/Descriptors/DescriptorSetHandler.hpp"

namespace mtd
{
    // Centralized handler for all mesh data in the scene
    class MeshPool
    {
        public:
            MeshPool() = default;
            ~MeshPool() = default;

            MeshPool(const MeshPool&) = delete;
            MeshPool& operator=(const MeshPool&) = delete;

            // Getters
            const MeshData& getMesh(uint32_t meshID) const { return meshes[meshID]; }
            ResourceID getVertexBufferID() const { return vertexBufferID; }
            ResourceID getIndexBufferID() const { return indexBufferID; }
            ResourceID getSubmeshBufferID() const { return submeshBufferID; }

            // Loads all scene mesh data to the GPU
            void loadMeshes(ResourceManager& resourceManager, const std::vector<std::string>& meshFiles);

        private:
            // GPU buffer ID for the vertex data
            ResourceID vertexBufferID = 0U;
            // GPU buffer ID for the index data
            ResourceID indexBufferID = 0U;
            // GPU buffer ID for mapping each mesh to its submeshes
            ResourceID submeshBufferID = 0U;

            // Mesh data for the CPU side
            std::vector<MeshData> meshes;

            // Loads all the mesh data from the CPU to the GPU
            void loadToGpu
            (
                ResourceManager& resourceManager,
                const std::vector<std::byte>& vertexData,
                const std::vector<uint32_t>& indexData
            );
    };
}
