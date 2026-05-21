#pragma once

#include "../Vulkan/Descriptors/DescriptorSetHandler.hpp"

namespace mtd
{
    // Centralized handler for all mesh data in the scene
    class MeshPool
    {
        public:
            MeshPool(const Device& mtdDevice);
            ~MeshPool() = default;

            MeshPool(const MeshPool&) = delete;
            MeshPool& operator=(const MeshPool&) = delete;

            // Loads all scene mesh data to the GPU
            void loadMeshes(const std::vector<std::string>& meshFiles);

            // Creates the descriptors for the mesh data
            void createMeshDescriptors
            (
                DescriptorSetHandler& descriptorSetHandler,
                uint32_t vertexBinding, uint32_t indexBinding
            ) const;

        private:
            // GPU buffer for the vertex data
            GpuBuffer vertexBuffer;
            // GPU buffer for the index data
            GpuBuffer indexBuffer;
            // Mesh pool command handler
			CommandHandler commandHandler;

            // Submeshes information
            std::vector<SubmeshData> submeshes;

            // Flag to indicate if the GPU buffers have been created
            bool gpuBuffersCreated = false;
            // Flag to indicate if there is any mesh was loaded to the GPU
            bool anyMeshLoaded = false;

            // Loads all the mesh data from the CPU to the GPU
            void loadToGpu(const std::vector<std::byte>& vertexData, const std::vector<uint32_t>& indexData);
    };
}
