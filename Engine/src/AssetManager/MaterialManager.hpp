#pragma once

#include "../Vulkan/Descriptors/DescriptorSetHandler.hpp"

namespace mtd
{
    // Handles all materials used in the scene
    class MaterialManager
    {
        public:
            MaterialManager(const Device& device);
            ~MaterialManager() = default;

            MaterialManager(const MaterialManager&) = delete;
            MaterialManager& operator=(const MaterialManager&) = delete;

            // Loads all scene materials and material sets to the GPU
            void loadMaterials
            (
                const std::vector<std::string>& materialFiles, const std::vector<std::vector<uint32_t>>& sets
            );

            // Creates the descriptors for the material indexing and material data
            void createMaterialDescriptor
            (
                DescriptorSetHandler& descriptorSetHandler,
                uint32_t materialDataBinding,
                uint32_t materialIndexingBinding,
                uint32_t materialSetBinding
            ) const;

        private:
            // GPU buffer for the material data
            GpuBuffer materialBuffer;
            // GPU buffer for the material indexing
            GpuBuffer materialIndexingBuffer;
            // GPU buffer for the material sets
            GpuBuffer materialSetBuffer;
            // Command handler for the GPU buffers
            CommandHandler commandHandler;

            // Flag to indicate if the GPU buffers have been created
            bool gpuBuffersCreated = false;
            // Flag to indicate if there is any material loaded in the GPU
            bool anyMaterialLoaded = false;

            // Loads all the material data, indexing and sets to the GPU
            void loadToGpu
            (
                const std::vector<std::byte>& materialData,
                const std::vector<uint32_t>& materialIndexing,
                const std::vector<uint32_t>& materialSetData
            );
    };
}
