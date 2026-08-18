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
            vk::DescriptorSetLayout getLayout(DescriptorLayoutID id) const;
            vk::DescriptorSet getSet(DescriptorSetID id) const;

            // Creates a descriptor set layout
            DescriptorLayoutID createLayout(const std::vector<DescriptorLayoutBindingInfo>& bindingInfos);
            // Creates the descriptor pool
            void createPool();

            // Creates and allocates a descriptor set
            DescriptorSetID allocateSet(DescriptorSetInfo& descriptorSetInfo, uint32_t count = 1U);

            // Write all descriptors from all descriptor sets
            void writeAll();
            // Write a specific descriptor binding
            void write(DescriptorSetID setID, uint32_t binding);

            // Updates all descriptors associated with the specified resource
            void updateResourceDescriptors(ResourceID resourceID);

            // Clears all descriptor data stored
            void clear();

        private:
            // Descriptor set layouts
            std::vector<DescriptorLayoutData> layouts;
            // Descriptor sets
            std::vector<DescriptorSetData> sets;

            // Descriptor pool to allocate the descriptor sets
            vk::DescriptorPool pool;

            // Registry of which descriptors are associated with a specific resource
            std::unordered_map<ResourceID, std::vector<DescriptorIdentifier>> resourceDescriptorsMap;

            // Number of descriptor bindings from all descriptor layouts
            size_t totalBindingsCount = 0;
            // Number of buffer resource references from all created descriptor sets
            size_t totalBufferResourcesCount = 0;
            // Number of image resource references from all created descriptor sets
            size_t totalImageResourcesCount = 0;
            // Register of descriptors to be allocated in the pool
            std::unordered_map<vk::DescriptorType, uint32_t> totalDescriptorTypeCount;

            // Device reference
            const Device& mtdDevice;
            // Reference to the engine's resource manager
            const ResourceManager& resourceManager;

            // Builds the write operation for a specific descriptor
            void buildWriteOperation
            (
                DescriptorSetID setID, uint32_t binding, vk::WriteDescriptorSet& writeOperation,
                std::vector<vk::DescriptorBufferInfo>& bufferInfos, std::vector<vk::DescriptorImageInfo>& imageInfos
            ) const;
    };
}
