#pragma once

#include <meltdown/event.hpp>

#include "../Vulkan/Device/GpuBuffer.hpp"
#include "../Vulkan/Image/Image.hpp"

namespace mtd
{
    // Centralized manager for GPU resources
    class ResourceManager
    {
        public:
            ResourceManager(const Device& mtdDevice, UIntVec2 windowResolution);
            ~ResourceManager() = default;

            ResourceManager(const ResourceManager&) = delete;
            ResourceManager& operator=(const ResourceManager&) = delete;

            // Getters
            ResourceID getResourceID(std::string_view resourceName) const;
            vk::Buffer getVulkanBuffer(ResourceID bufferID) const;
            uint64_t getBufferSize(ResourceID bufferID) const;

            // Creates a new GPU buffer
            ResourceID createBuffer
            (
                std::string resourceName,
                GpuBufferType type,
                GpuMemoryUsage memoryUsage = GpuMemoryUsage::Auto,
                uint64_t bufferSize = 0UL,
                const void* pData = nullptr
            );
            // Creates a new image on the GPU
            ResourceID createImage
            (
                std::string resourceName,
                UIntVec2 imageDimensions,
				vk::Format imageFormat,
				vk::ImageUsageFlags usage,
                Vec2 windowResolutionRatio = Vec2{-1.0f, -1.0f},
                SamplerType samplerType = SamplerType::Linear,
				vk::ImageTiling tiling = vk::ImageTiling::eOptimal,
                vk::ImageAspectFlags aspects = vk::ImageAspectFlagBits::eColor,
                vk::ImageViewType viewType = vk::ImageViewType::e2D,
                vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
            );
            // Creates loads an image and creates the GPU resource from it
            ResourceID loadImage
            (
                std::string resourceName,
                UIntVec2 imageDimensions,
                const void* pData,
                uint64_t dataSize,
				vk::Format imageFormat,
				vk::ImageUsageFlags usage,
                SamplerType samplerType = SamplerType::Linear,
				vk::ImageTiling tiling = vk::ImageTiling::eOptimal,
                vk::ImageAspectFlags aspects = vk::ImageAspectFlagBits::eColor,
                vk::ImageViewType viewType = vk::ImageViewType::e2D,
                vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
            );

            // Updates buffer data
            bool updateBufferData(ResourceID id, uint64_t copySize, const void* srcData, uint64_t bufferOffset = 0UL);
            // Resizes the specified GPU buffer
            bool resizeBuffer(ResourceID id, uint64_t newSize);

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

            // Fetches the descriptor buffer info for the specified buffer data
            bool fetchDescriptorBufferInfo(ResourceID id, vk::DescriptorBufferInfo& info) const;
            // Fetches the descriptor image info for the specified image data
            bool fetchDescriptorImageInfo(ResourceID id, vk::DescriptorImageInfo& info) const;

        private:
            // GPU resources
            std::unordered_map<ResourceID, GpuBuffer> buffers;
            std::unordered_map<ResourceID, Image> images;

            // Map linking the resource name to its ID
            std::unordered_map<std::string, ResourceID> nameIdMap;
            // Resource ID counter
            ResourceID nextID = 1U;

            // Window resolution saved for creating images associated with the window resolution
            UIntVec2 windowResolution;

            // Resource manager's command handler
            CommandHandler commandHandler;

            // Callback handle to update buffers as requested
            EventCallbackHandle updateGpuBufferHandle;

            // Device reference
            const Device& mtdDevice;

            // Configures the event callbacks
            void configureEventCallbacks();
    };
}
