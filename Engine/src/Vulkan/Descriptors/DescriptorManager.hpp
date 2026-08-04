#pragma once
#include "DescriptorPool.hpp"

#include "../../AssetManager/ResourceManager.hpp"

namespace mtd
{
    // Centralized manager for all descriptors
    class DescriptorManager
    {
        public:
            DescriptorManager(const Device& mtdDevice, const ResourceManager& resourceManager);
            ~DescriptorManager();

            DescriptorManager(const DescriptorManager&) = delete;
            DescriptorManager& operator=(const DescriptorManager&) = delete;

            // Getters
            vk::DescriptorSetLayout getLayout(DescriptorSetID id) const;
            vk::DescriptorSet getSet(DescriptorSetID id) const;

            // Creates a descriptor layout and set
            DescriptorSetID createSet(const std::vector<DescriptorInfo>& descriptorInfos);

            // Allocates and writes all descriptor sets in the pool
            void allocate();

            // Clears all descriptor data stored
            void clear();

        private:
            // Descriptor sets
            std::vector<DescriptorSetHandler> sets;
            // Descriptor pool to allocate the descriptor sets
            vk::DescriptorPool pool;

            // Register of descriptors to be allocated in the pool
            std::unordered_map<vk::DescriptorType, uint32_t> totalDescriptorTypeCount;

            // Device reference
            const Device& mtdDevice;
            // Reference to the engine's resource manager
            const ResourceManager& resourceManager;

            // Creates the descriptor pool
            void createPool();
    };
}
