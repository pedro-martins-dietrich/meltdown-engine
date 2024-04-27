#pragma once

#include "../Command/CommandHandler.hpp"

namespace mtd::Image
{
	struct CreateImageBundle
	{
		const vk::Device& device;
		vk::ImageTiling tiling;
		vk::ImageUsageFlags usage;
		vk::Format format;
		vk::ImageCreateFlags imageFlags;
		FrameDimensions dimensions;
	};

	// Creates a Vulkan image
	void createImage(const CreateImageBundle& createBundle, vk::Image& image);
	// Allocates GPU memory for Vulkan image
	void createImageMemory
	(
		const Device& device,
		const vk::Image& image,
		vk::MemoryPropertyFlags memoryProperties,
		vk::DeviceMemory& imageMemory
	);
	// Creates a description for the Vulkan image
	void createImageView
	(
		const vk::Device& device,
		const vk::Image& image,
		vk::Format format,
		vk::ImageAspectFlags aspect,
		vk::ImageViewType viewType,
		vk::ImageView& view
	);
}
