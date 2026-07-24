#pragma once

#include "../Vulkan/Device/GpuBuffer.hpp"
#include "../Vulkan/Image/Image.hpp"

namespace mtd
{
    using ResourceID = uint32_t;
    using Resource = std::variant<GpuBuffer, Image>;

    // Centralized manager for GPU resources
    class ResourceManager
    {
        public:
            ResourceManager(const Device& device);
            ~ResourceManager() = default;

            ResourceManager(const ResourceManager&) = delete;
            ResourceManager& operator=(const ResourceManager&) = delete;

            // Creates a new GPU buffer
            ResourceID createBuffer
            (
                vk::BufferUsageFlags usage,
                vk::MemoryPropertyFlags memoryProperties,
                vk::DeviceSize bufferSize = 0U
            );
            // Creates a new image on the GPU
            ResourceID createImage
            (
                UIntVec2 imageDimensions,
				vk::Format imageFormat,
				vk::ImageTiling tiling,
				vk::ImageUsageFlags usage,
                vk::MemoryPropertyFlags memoryProperties,
                vk::ImageAspectFlags aspect,
                vk::ImageViewType viewType,
                SamplerType samplerType = SamplerType::Linear
            );

            // Updates buffer data
            bool updateBufferData
            (
                ResourceID id, vk::DeviceSize copySize, const void* srcData, vk::DeviceSize bufferOffset = 0
            );
            // Transitions the specified image layout
            bool transitionImageLayout
            (
                ResourceID id,
                vk::CommandBuffer commandBuffer,
				vk::ImageLayout newLayout,
				vk::PipelineStageFlags srcStage = vk::PipelineStageFlagBits::eNone,
				vk::PipelineStageFlags dstStage = vk::PipelineStageFlagBits::eNone
            ) const;

            // Deletes the specified GPU resource
            bool deleteResource(ResourceID id);
            // Deletes all resources and resets the ID counter
            void clearResources();

            // Updates the descriptor buffer info with the specified buffer data
            bool updateDescriptorBufferInfo(ResourceID id, vk::DescriptorBufferInfo& info) const;
            // Updates the descriptor image info with the specified image data
            bool updateDescriptorImageInfo(ResourceID id, vk::DescriptorImageInfo& info) const;

        private:
            // GPU resources
            std::unordered_map<ResourceID, Resource> resources;

            // Resource ID counter
            ResourceID nextID = 1U;

            // Device reference
            const Device& device;
    };
}
