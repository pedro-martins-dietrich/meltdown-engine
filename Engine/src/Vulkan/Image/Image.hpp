#pragma once

#include "SamplerManager.hpp"
#include "../Command/CommandHandler.hpp"

namespace mtd
{
	// Handles the properties and the GPU memory associated with a Vulkan image
	class Image
	{
		public:
			Image(const Device& mtdDevice);
			Image
			(
				const Device& mtdDevice,
				UIntVec2 imageDimensions,
				vk::Format imageFormat,
				vk::ImageTiling imageTiling,
				vk::ImageUsageFlags usage,
				vk::MemoryPropertyFlags memoryPropertyFlags,
				vk::ImageAspectFlags aspects,
				vk::ImageViewType imageViewType = vk::ImageViewType::e2D,
				SamplerType sampler = SamplerType::Linear,
				Vec2 windowResolutionRatio = Vec2{-1.0f, -1.0f},
				vk::ImageCreateFlags imageFlags = vk::ImageCreateFlags()
			);
			~Image();

			Image(const Image&) = delete;
			Image& operator=(const Image&) = delete;

			Image(Image&& other) noexcept;

			// Getters
			vk::Image getImage() const { return image; }
			vk::ImageView getView() const { return view; }
			vk::Format getFormat() const { return format; }
			UIntVec2 getDimensions() const { return dimensions; }
			Vec2 getWindowResolutionRatio() const { return windowResolutionRatio; }

			// Creates the Vulkan image, image memory and image view of the resource
			void create
			(
				UIntVec2 imageDimensions,
				vk::Format imageFormat,
				vk::ImageTiling imageTiling,
				vk::ImageUsageFlags usage,
				vk::MemoryPropertyFlags memoryPropertyFlags,
				vk::ImageAspectFlags aspects,
				vk::ImageViewType imageViewType = vk::ImageViewType::e2D,
				SamplerType sampler = SamplerType::Linear,
				vk::ImageCreateFlags imageFlags = vk::ImageCreateFlags()
			);

			// Recreates the image, image memory and image view with a new resolution
			void resize(UIntVec2 newDimensions);

			// Updates the descriptor image info with the image data
			void updateDescriptorInfo(vk::DescriptorImageInfo& descriptorImageInfo) const;

			// Changes the Vulkan image layout
			void transitionImageLayout
			(
				vk::CommandBuffer commandBuffer,
				vk::ImageLayout newLayout,
				vk::PipelineStageFlags srcStage = vk::PipelineStageFlagBits::eNone,
				vk::PipelineStageFlags dstStage = vk::PipelineStageFlagBits::eNone
			) const;
			// Copies buffer data to Vulkan image
			void copyBufferToImage(const CommandHandler& commandHandler, vk::Buffer srcBuffer);

		private:
			// Vulkan image data
			vk::Image image;
			// GPU memory region of the image
			vk::DeviceMemory imageMemory;
			// Image description
			vk::ImageView view;

			// Image resolution
			UIntVec2 dimensions = UIntVec2{0U, 0U};
			// Image pixel format
			vk::Format format = vk::Format::eUndefined;
			// Image tiling
			vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
			// Image usages
			vk::ImageUsageFlags usageFlags = vk::ImageUsageFlags();
			// Image memory properties
			vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
			// Image aspects
			vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlags();
			// Image view type
			vk::ImageViewType viewType = vk::ImageViewType::e2D;
			// Method for image sampling
			SamplerType samplerType = SamplerType::Linear;
			// Image creation flags
			vk::ImageCreateFlags createFlags = vk::ImageCreateFlags();
			// Image GPU memory layout
			mutable vk::ImageLayout layout = vk::ImageLayout::eUndefined;

			// Ratio between the image resolution and the window resolution
			Vec2 windowResolutionRatio = Vec2{-1.0f, -1.0f};

			// Device reference
			const Device& mtdDevice;

			// Creates the Vulkan image
			void createImage();
			// Allocates GPU memory for Vulkan image
			void createMemory();
			// Creates a description for the Vulkan image
			void createView();
	};
}
