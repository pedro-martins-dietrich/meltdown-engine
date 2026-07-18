#pragma once

#include "../Command/CommandHandler.hpp"

namespace mtd
{
	// Handles the properties and the GPU memory associated with a Vulkan image
	class Image
	{
		public:
			Image(const vk::Device& device);
			Image
			(
				const Device& mtdDevice,
				UIntVec2 dimensions,
				vk::Format imageFormat,
				vk::ImageTiling tiling,
				vk::ImageUsageFlags usage,
				vk::MemoryPropertyFlags memoryProperties,
				vk::ImageAspectFlags aspect,
				vk::ImageViewType viewType,
				vk::Filter samplingFilter,
				vk::ImageCreateFlags imageFlags = vk::ImageCreateFlags()
			);
			~Image();

			Image(const Image&) = delete;
			Image& operator=(const Image&) = delete;

			Image(Image&& other) noexcept;

			// Getters
			vk::Image getImage() const { return image; }
			vk::ImageView getImageView() const { return view; }
			vk::Format getFormat() const { return format; }
			UIntVec2 getDimensions() const { return dimensions; }

			// Setter
			void setVulkanImage(vk::Image newImage, vk::Format newFormat, UIntVec2 newDimensions);

			// Creates the Vulkan image
			void createImage
			(
				UIntVec2 imageDimensions,
				vk::Format imageFormat,
				vk::ImageTiling tiling,
				vk::ImageUsageFlags usage,
				vk::ImageCreateFlags imageFlags = vk::ImageCreateFlags()
			);
			// Allocates GPU memory for Vulkan image
			void createImageMemory(const Device& mtdDevice, vk::MemoryPropertyFlags memoryProperties);
			// Creates a description for the Vulkan image
			void createImageView(vk::ImageAspectFlags aspect, vk::ImageViewType viewType);
			// Creates a sampler for the image
			void createImageSampler(vk::Filter samplingFilter);

			// Recreates the image, image memory and image view with a new resolution
			void resize
			(
				const Device& mtdDevice,
				UIntVec2 newDimensions,
				vk::ImageTiling tiling,
				vk::ImageUsageFlags usage,
				vk::MemoryPropertyFlags memoryProperties,
				vk::ImageAspectFlags aspect,
				vk::ImageViewType viewType,
				vk::ImageCreateFlags imageFlags = vk::ImageCreateFlags()
			);

			// Updates the descriptor image info with the image data
			void updateDescriptorInfo(vk::DescriptorImageInfo& descriptorImageInfo) const;
			void defineDescriptorImageInfo(vk::DescriptorImageInfo* descriptorImageInfo) const
				{ updateDescriptorInfo(*descriptorImageInfo); } // TODO: Remove this version of the function

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

			// Deletes all the image data
			void destroy();

		private:
			// Vulkan image data
			vk::Image image;
			// GPU memory region of the image
			vk::DeviceMemory imageMemory;
			// Image description
			vk::ImageView view;
			// Method for image sampling
			vk::Sampler sampler;

			// Image resolution
			UIntVec2 dimensions = UIntVec2{0U, 0U};
			// Image pixel format
			vk::Format format = vk::Format::eUndefined;
			// Image GPU memory layout
			mutable vk::ImageLayout layout = vk::ImageLayout::eUndefined;

			// Vulkan device reference
			const vk::Device& device;
	};
}
