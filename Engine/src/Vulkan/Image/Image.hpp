#pragma once

#include "../Command/CommandHandler.hpp"

namespace mtd
{
	// Handles the properties and the GPU memory associated with a Vulkan image
	class Image
	{
		public:
			Image(const vk::Device& device);
			~Image();

			Image(const Image&) = delete;
			Image& operator=(const Image&) = delete;

			Image(Image&& other) noexcept;

			// Getters
			vk::Image getImage() const { return image; }
			vk::ImageView getImageView() const { return view; }
			vk::Format getFormat() const { return format; }

			// Setter
			void setVulkanImage(vk::Image newImage, vk::Format newFormat, FrameDimensions newDimensions);

			// Creates the Vulkan image
			void createImage
			(
				FrameDimensions imageDimensions,
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
				FrameDimensions newDimensions,
				vk::ImageTiling tiling,
				vk::ImageUsageFlags usage,
				vk::MemoryPropertyFlags memoryProperties,
				vk::ImageAspectFlags aspect,
				vk::ImageViewType viewType,
				vk::ImageCreateFlags imageFlags = vk::ImageCreateFlags()
			);

			// Defines the fields of the descriptor image info with the image data
			void defineDescriptorImageInfo(vk::DescriptorImageInfo* descriptorImageInfo) const;

			// Changes the Vulkan image layout
			void transitionImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout newLayout) const;
			// Copies buffer data to Vulkan image
			void copyBufferToImage(const CommandHandler& commandHandler, vk::Buffer srcBuffer);

			// Deletes all the image data
			void destroy();

		private:
			// Vulkan image data
			vk::Image image;
			// GPU memory of the image
			vk::DeviceMemory imageMemory;
			// Image description
			vk::ImageView view;
			// Method for image sampling
			vk::Sampler sampler;

			// Image pixel format
			vk::Format format;
			// Image GPU memory layout
			mutable vk::ImageLayout layout;
			// Image resolution
			FrameDimensions dimensions;

			// Vulkan device reference
			const vk::Device& device;
	};
}
