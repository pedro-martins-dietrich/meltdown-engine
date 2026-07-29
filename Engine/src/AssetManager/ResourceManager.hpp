#pragma once

#include "../Vulkan/Device/GpuBuffer.hpp"
#include "../Vulkan/Image/Image.hpp"

namespace mtd
{
    // ID used for all GPU resources
    using ResourceID = uint32_t;

    // Centralized manager for GPU resources
    class ResourceManager
    {
        public:
            ResourceManager(const Device& device, UIntVec2 windowResolution);
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
				vk::ImageUsageFlags usage,
                Vec2 windowResolutionRatio = Vec2{-1.0f, -1.0f},
                SamplerType samplerType = SamplerType::Linear,
				vk::ImageTiling tiling = vk::ImageTiling::eOptimal,
                vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor,
                vk::ImageViewType viewType = vk::ImageViewType::e2D,
                vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
            );

            // Updates buffer data
            bool updateBufferData
            (
                ResourceID id, vk::DeviceSize copySize, const void* srcData, vk::DeviceSize bufferOffset = 0UL
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

            // Updates resolution of all images where the resolution is linked to the window size
            void updateWindowResolutionLinkedImages(UIntVec2 newWindowResolution);

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
            std::unordered_map<ResourceID, GpuBuffer> buffers;
            std::unordered_map<ResourceID, Image> images;

            // Resource ID counter
            ResourceID nextID = 1U;

            // Window resolution saved for creating images associated with the window resolution
            UIntVec2 windowResolution;

            // Device reference
            const Device& device;
    };
}
