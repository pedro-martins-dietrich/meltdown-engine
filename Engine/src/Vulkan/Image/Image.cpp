#include <pch.hpp>
#include "Image.hpp"

#include "../Device/GpuBuffer.hpp"
#include "../../Utils/Logger.hpp"

// Creates a Vulkan image
void mtd::Image::createImage(const CreateImageBundle& createBundle, vk::Image& image)
{
	vk::ImageCreateInfo imageCreateInfo{};
	imageCreateInfo.flags = createBundle.imageFlags;
	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.format = createBundle.format;
	imageCreateInfo.extent = vk::Extent3D{createBundle.dimensions.width, createBundle.dimensions.height, 1};
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
	imageCreateInfo.tiling = createBundle.tiling;
	imageCreateInfo.usage = createBundle.usage;
	imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;

	vk::Result result = createBundle.device.createImage(&imageCreateInfo, nullptr, &image);
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

// Changes the Vulkan image layout
void mtd::Image::transitionImageLayout
(
	const vk::Image& image,
	const vk::CommandBuffer& commandBuffer,
	vk::ImageLayout oldLayout,
	vk::ImageLayout newLayout
)
{
	vk::ImageSubresourceRange subresource{};
	subresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	subresource.baseMipLevel = 0;
	subresource.levelCount = 1;
	subresource.baseArrayLayer = 0;
	subresource.layerCount = 1;

	vk::ImageMemoryBarrier barrier{};
	barrier.srcAccessMask = vk::AccessFlagBits::eNone;
	barrier.dstAccessMask = vk::AccessFlagBits::eNone;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
	barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
	barrier.image = image;
	barrier.subresourceRange = subresource;

	vk::PipelineStageFlags srcStage, dstStage;
	switch(oldLayout)
	{
		case vk::ImageLayout::eUndefined:
			barrier.srcAccessMask = vk::AccessFlagBits::eNone;
			srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			break;
		case vk::ImageLayout::eTransferDstOptimal:
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			srcStage = vk::PipelineStageFlagBits::eTransfer;
			break;
		case vk::ImageLayout::eGeneral:
			barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
			srcStage = vk::PipelineStageFlagBits::eRayTracingShaderKHR;
			break;
		default:
			LOG_WARNING("Unexpected source image layout for transition: %d.", oldLayout);
			srcStage = vk::PipelineStageFlagBits::eNone;
	}
	switch(newLayout)
	{
		case vk::ImageLayout::eTransferDstOptimal:
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
			dstStage = vk::PipelineStageFlagBits::eTransfer;
			break;
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader;
			break;
		case vk::ImageLayout::eGeneral:
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
			dstStage = vk::PipelineStageFlagBits::eRayTracingShaderKHR;
			break;
		default:
			LOG_WARNING("Unexpected destination image layout for transition: %d.", newLayout);
			dstStage = vk::PipelineStageFlagBits::eNone;
	}

	commandBuffer.pipelineBarrier(srcStage, dstStage, vk::DependencyFlags(), nullptr, nullptr, barrier);
}

// Copies buffer data to Vulkan image
void mtd::Image::copyBufferToImage
(
	const vk::Image& image,
	const CommandHandler& commandHandler,
	vk::Buffer srcBuffer,
	FrameDimensions dimensions
)
{
	vk::ImageSubresourceLayers subresource{};
	subresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	subresource.mipLevel = 0;
	subresource.baseArrayLayer = 0;
	subresource.layerCount = 1;

	vk::BufferImageCopy bufferImageCopy{};
	bufferImageCopy.bufferOffset = 0;
	bufferImageCopy.bufferRowLength = 0;
	bufferImageCopy.bufferImageHeight = 0;
	bufferImageCopy.imageSubresource = subresource;
	bufferImageCopy.imageOffset = vk::Offset3D{0, 0, 0};
	bufferImageCopy.imageExtent = vk::Extent3D{dimensions.width, dimensions.height, 1};

	vk::CommandBuffer commandBuffer = commandHandler.beginSingleTimeCommand();

	commandBuffer.copyBufferToImage
	(
		srcBuffer,
		image,
		vk::ImageLayout::eTransferDstOptimal,
		bufferImageCopy
	);

	commandHandler.endSingleTimeCommand(commandBuffer);
}
