#include "Image.hpp"

#include "../Device/Memory.hpp"
#include "../../Utils/Logger.hpp"

// Creates a Vulkan image
void mtd::Image::createImage(const CreateImageBundle& createBundle, vk::Image& image)
{
	vk::ImageCreateInfo imageCreateInfo{};
	imageCreateInfo.flags = createBundle.imageFlags;
	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.format = createBundle.format;
	imageCreateInfo.extent =
		vk::Extent3D{createBundle.dimensions.width, createBundle.dimensions.height, 1};
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
	imageCreateInfo.tiling = createBundle.tiling;
	imageCreateInfo.usage = createBundle.usage;
	imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;

	vk::Result result =
		createBundle.device.createImage(&imageCreateInfo, nullptr, &image);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create image. Vulkan result: %d", result);
}

// Allocates GPU memory for Vulkan image
void mtd::Image::createImageMemory
(
	const Device& device,
	const vk::Image& image,
	vk::MemoryPropertyFlags memoryProperties,
	vk::DeviceMemory& imageMemory
)
{
	const vk::Device& vulkanDevice = device.getDevice();

	vk::MemoryRequirements requirements = vulkanDevice.getImageMemoryRequirements(image);

	vk::MemoryAllocateInfo allocationInfo{};
	allocationInfo.allocationSize = requirements.size;
	allocationInfo.memoryTypeIndex = Memory::findMemoryTypeIndex
	(
		device.getPhysicalDevice(),
		requirements.memoryTypeBits,
		memoryProperties
	);

	vk::Result result = vulkanDevice.allocateMemory(&allocationInfo, nullptr, &imageMemory);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to allocate memory for image. Vulkan result: %d", result);
		return;
	}

	vulkanDevice.bindImageMemory(image, imageMemory, 0);
}

// Creates a description for the Vulkan image
void mtd::Image::createImageView
(
	const vk::Device& device,
	const vk::Image& image,
	vk::Format format,
	vk::ImageAspectFlags aspect,
	vk::ImageViewType viewType,
	vk::ImageView& view
)
{
	vk::ComponentMapping componentMapping{};
	componentMapping.r = vk::ComponentSwizzle::eIdentity;
	componentMapping.g = vk::ComponentSwizzle::eIdentity;
	componentMapping.b = vk::ComponentSwizzle::eIdentity;
	componentMapping.a = vk::ComponentSwizzle::eIdentity;

	vk::ImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = aspect;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	vk::ImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.flags = vk::ImageViewCreateFlags();
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = viewType;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components = componentMapping;
	imageViewCreateInfo.subresourceRange = subresourceRange;

	vk::Result result = device.createImageView(&imageViewCreateInfo, nullptr, &view);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create image view. Vulkan result: %d", result);
}
