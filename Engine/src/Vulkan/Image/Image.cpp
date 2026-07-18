#include <pch.hpp>
#include "Image.hpp"

#include "../Device/GpuBuffer.hpp"
#include "../../Utils/Logger.hpp"

mtd::Image::Image(const vk::Device& device)
	: device{device}, image{nullptr}, imageMemory{nullptr}, view{nullptr}, sampler{nullptr}
{}

mtd::Image::Image
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
	vk::ImageCreateFlags imageFlags
) : device{mtdDevice.getDevice()}, image{nullptr}, imageMemory{nullptr}, view{nullptr}, sampler{nullptr}
{
	createImage(dimensions, imageFormat, tiling, usage, imageFlags);
	createImageMemory(mtdDevice, memoryProperties);
	createImageView(aspect, viewType);
	createImageSampler(samplingFilter);
}

mtd::Image::~Image()
{
	device.destroySampler(sampler);
	device.destroyImageView(view);
	device.freeMemory(imageMemory);
	device.destroyImage(image);
}

mtd::Image::Image(Image&& other) noexcept
	: device{other.device},
	image{std::move(other.image)},
	imageMemory{std::move(other.imageMemory)},
	view{std::move(other.view)},
	sampler{std::move(other.sampler)},
	format{other.format},
	layout{other.layout},
	dimensions{other.dimensions}
{
	other.image = nullptr;
	other.imageMemory = nullptr;
	other.view = nullptr;
	other.sampler = nullptr;
}

void mtd::Image::setVulkanImage(vk::Image newImage, vk::Format newFormat, UIntVec2 newDimensions)
{
	image = newImage;
	format = newFormat;
	dimensions = newDimensions;
}

void mtd::Image::createImage
(
	UIntVec2 imageDimensions,
	vk::Format imageFormat,
	vk::ImageTiling tiling,
	vk::ImageUsageFlags usage,
	vk::ImageCreateFlags imageFlags
)
{
	assert(image == nullptr && "The Vulkan image has already been created.");

	format = imageFormat;
	dimensions = imageDimensions;

	vk::ImageCreateInfo imageCreateInfo{};
	imageCreateInfo.flags = imageFlags;
	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent = vk::Extent3D{dimensions.x, dimensions.y, 1U};
	imageCreateInfo.mipLevels = 1U;
	imageCreateInfo.arrayLayers = 1U;
	imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.usage = usage;
	imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	imageCreateInfo.queueFamilyIndexCount = 0U;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = layout;

	vk::Result result = device.createImage(&imageCreateInfo, nullptr, &image);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create image. Vulkan result: %d", result);
}

void mtd::Image::createImageMemory(const Device& mtdDevice, vk::MemoryPropertyFlags memoryProperties)
{
	assert(image != nullptr && "The image must be created before the image memory.");
	assert(imageMemory == nullptr && "The Vulkan image memory has already been created.");

	vk::MemoryRequirements requirements = device.getImageMemoryRequirements(image);

	vk::MemoryAllocateInfo allocationInfo{};
	allocationInfo.allocationSize = requirements.size;
	allocationInfo.memoryTypeIndex = Memory::findMemoryTypeIndex
	(
		mtdDevice.getPhysicalDevice(),
		requirements.memoryTypeBits,
		memoryProperties
	);

	vk::Result result = device.allocateMemory(&allocationInfo, nullptr, &imageMemory);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to allocate memory for image. Vulkan result: %d", result);
		return;
	}

	device.bindImageMemory(image, imageMemory, 0);
}

void mtd::Image::createImageView(vk::ImageAspectFlags aspect, vk::ImageViewType viewType)
{
	assert(image != nullptr && "The image must be created before the image view.");
	assert(view == nullptr && "The Vulkan image view has already been created.");

	vk::ComponentMapping componentMapping{};
	componentMapping.r = vk::ComponentSwizzle::eIdentity;
	componentMapping.g = vk::ComponentSwizzle::eIdentity;
	componentMapping.b = vk::ComponentSwizzle::eIdentity;
	componentMapping.a = vk::ComponentSwizzle::eIdentity;

	vk::ImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = aspect;
	subresourceRange.baseMipLevel = 0U;
	subresourceRange.levelCount = 1U;
	subresourceRange.baseArrayLayer = 0U;
	subresourceRange.layerCount = 1U;

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

void mtd::Image::createImageSampler(vk::Filter samplingFilter)
{
	assert(sampler == nullptr && "The Vulkan sampler has already been created.");

	vk::SamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.flags = vk::SamplerCreateFlags();
	samplerCreateInfo.magFilter = samplingFilter;
	samplerCreateInfo.minFilter = samplingFilter;
	samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = vk::False;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	samplerCreateInfo.compareEnable = vk::False;
	samplerCreateInfo.compareOp = vk::CompareOp::eAlways;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerCreateInfo.unnormalizedCoordinates = vk::False;

	vk::Result result = device.createSampler(&samplerCreateInfo, nullptr, &sampler);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create texture sampler. Vulkan result: %d", result);
}

void mtd::Image::resize
(
	const Device& mtdDevice,
	UIntVec2 newDimensions,
	vk::ImageTiling tiling,
	vk::ImageUsageFlags usage,
	vk::MemoryPropertyFlags memoryProperties,
	vk::ImageAspectFlags aspect,
	vk::ImageViewType viewType,
	vk::ImageCreateFlags imageFlags
)
{
	device.destroyImageView(view);
	device.freeMemory(imageMemory);
	device.destroyImage(image);

	image = nullptr;
	imageMemory = nullptr;
	view = nullptr;
	layout = vk::ImageLayout::eUndefined;

	createImage(newDimensions, format, tiling, usage, imageFlags);
	createImageMemory(mtdDevice, memoryProperties);
	createImageView(aspect, viewType);
}

void mtd::Image::updateDescriptorInfo(vk::DescriptorImageInfo& descriptorImageInfo) const
{
	assert(view != nullptr && "The image and its view must be created before updating the descriptor image info.");

	descriptorImageInfo.sampler = sampler;
	descriptorImageInfo.imageView = view;
	descriptorImageInfo.imageLayout = layout;
}

void mtd::Image::transitionImageLayout
(
	vk::CommandBuffer commandBuffer,
	vk::ImageLayout newLayout,
	vk::PipelineStageFlags srcStage,
	vk::PipelineStageFlags dstStage
) const
{
	assert(image != nullptr && "The image must be created before transitioning it.");

	if(newLayout == layout) return;

	vk::ImageSubresourceRange subresource{};
	subresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	subresource.baseMipLevel = 0U;
	subresource.levelCount = 1U;
	subresource.baseArrayLayer = 0U;
	subresource.layerCount = 1U;

	vk::ImageMemoryBarrier barrier{};
	barrier.srcAccessMask = vk::AccessFlagBits::eNone;
	barrier.dstAccessMask = vk::AccessFlagBits::eNone;
	barrier.oldLayout = layout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
	barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
	barrier.image = image;
	barrier.subresourceRange = subresource;

	switch(layout)
	{
		case vk::ImageLayout::eUndefined:
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			barrier.srcAccessMask = vk::AccessFlagBits::eNone;
			srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			break;
		case vk::ImageLayout::eTransferDstOptimal:
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			srcStage = vk::PipelineStageFlagBits::eTransfer;
			break;
		case vk::ImageLayout::eGeneral:
			barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
			assert
			(
				srcStage != vk::PipelineStageFlagBits::eNone
				&& "The image source stage when transitioning from the general image layout must not be none."
			);
			break;
		default:
			LOG_WARNING("Unexpected image source layout for transition: %d.", layout);
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
			assert
			(
				dstStage != vk::PipelineStageFlagBits::eNone
				&& "The image destination stage when transitioning to the shader read only image layout must not be none."
			);
			break;
		case vk::ImageLayout::eGeneral:
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
			assert
			(
				dstStage != vk::PipelineStageFlagBits::eNone
				&& "The image destination stage when transitioning to the general image layout must not be none."
			);
			break;
		default:
			LOG_WARNING("Unexpected image destination layout for transition: %d.", newLayout);
			dstStage = vk::PipelineStageFlagBits::eNone;
	}

	commandBuffer.pipelineBarrier(srcStage, dstStage, vk::DependencyFlags(), nullptr, nullptr, barrier);
	layout = newLayout;
}

void mtd::Image::copyBufferToImage(const CommandHandler& commandHandler, vk::Buffer srcBuffer)
{
	assert(image != nullptr && "The image must be created before copying to it.");

	vk::ImageSubresourceLayers subresource{};
	subresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	subresource.mipLevel = 0U;
	subresource.baseArrayLayer = 0U;
	subresource.layerCount = 1U;

	vk::BufferImageCopy bufferImageCopy{};
	bufferImageCopy.bufferOffset = 0UL;
	bufferImageCopy.bufferRowLength = 0U;
	bufferImageCopy.bufferImageHeight = 0U;
	bufferImageCopy.imageSubresource = subresource;
	bufferImageCopy.imageOffset = vk::Offset3D{0, 0, 0};
	bufferImageCopy.imageExtent = vk::Extent3D{dimensions.x, dimensions.y, 1U};

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

void mtd::Image::destroy()
{
	device.destroySampler(sampler);
	device.destroyImageView(view);
	device.freeMemory(imageMemory);
	device.destroyImage(image);

	image = nullptr;
	imageMemory = nullptr;
	view = nullptr;
	sampler = nullptr;

	format = vk::Format::eUndefined;
	layout = vk::ImageLayout::eUndefined;
	dimensions = {0U, 0U};
}
