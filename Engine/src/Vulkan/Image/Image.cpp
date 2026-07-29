#include <pch.hpp>
#include "Image.hpp"

#include "../Device/GpuBuffer.hpp"
#include "../../Utils/Logger.hpp"

mtd::Image::Image(const Device& mtdDevice)
	: mtdDevice{mtdDevice}, image{nullptr}, imageMemory{nullptr}, view{nullptr}
{}

mtd::Image::Image
(
	const Device& mtdDevice,
	UIntVec2 imageDimensions,
	vk::Format imageFormat,
	vk::ImageTiling imageTiling,
	vk::ImageUsageFlags usage,
	vk::MemoryPropertyFlags memoryPropertyFlags,
	vk::ImageAspectFlags aspects,
	vk::ImageViewType imageViewType,
	SamplerType sampler,
	Vec2 windowResolutionRatio,
	vk::ImageCreateFlags imageFlags
) : mtdDevice{mtdDevice}, image{nullptr}, imageMemory{nullptr}, view{nullptr},
	dimensions{imageDimensions}, format{imageFormat}, tiling{imageTiling}, usageFlags{usage},
	memoryProperties{memoryPropertyFlags}, aspectFlags{aspects}, viewType{imageViewType},
	samplerType{sampler}, createFlags{imageFlags}, windowResolutionRatio{windowResolutionRatio}
{
	createImage();
	createMemory();
	createView();
}

mtd::Image::~Image()
{
	const vk::Device& device = mtdDevice.getDevice();
	device.destroyImageView(view);
	device.freeMemory(imageMemory);
	device.destroyImage(image);
}

mtd::Image::Image(Image&& other) noexcept
	: mtdDevice{other.mtdDevice},
	image{std::move(other.image)},
	imageMemory{std::move(other.imageMemory)},
	view{std::move(other.view)},
	dimensions{other.dimensions},
	format{other.format},
	tiling{other.tiling},
	usageFlags{other.usageFlags},
	memoryProperties{other.memoryProperties},
	aspectFlags{other.aspectFlags},
	viewType{other.viewType},
	samplerType{other.samplerType},
	createFlags{other.createFlags},
	layout{other.layout}
{
	other.image = nullptr;
	other.imageMemory = nullptr;
	other.view = nullptr;

	other.dimensions = UIntVec2{0U, 0U};
}

void mtd::Image::create
(
	UIntVec2 imageDimensions,
	vk::Format imageFormat,
	vk::ImageTiling imageTiling,
	vk::ImageUsageFlags usage,
	vk::MemoryPropertyFlags memoryPropertyFlags,
	vk::ImageAspectFlags aspects,
	vk::ImageViewType imageViewType,
	SamplerType sampler,
	vk::ImageCreateFlags imageFlags
)
{
	dimensions = imageDimensions;
	format = imageFormat;
	tiling = imageTiling;
	usageFlags = usage;
	memoryProperties = memoryPropertyFlags;
	aspectFlags = aspects;
	viewType = imageViewType;
	samplerType = sampler;
	createFlags = imageFlags;

	createImage();
	createMemory();
	createView();
}

void mtd::Image::resize(UIntVec2 newDimensions)
{
	assert(image && "The Vulkan image must be created before resizing.");

	mtdDevice.getDevice().destroyImageView(view);
	mtdDevice.getDevice().freeMemory(imageMemory);
	mtdDevice.getDevice().destroyImage(image);

	image = nullptr;
	imageMemory = nullptr;
	view = nullptr;
	layout = vk::ImageLayout::eUndefined;

	dimensions = newDimensions;

	createImage();
	createMemory();
	createView();
}

void mtd::Image::updateDescriptorInfo(vk::DescriptorImageInfo& descriptorImageInfo) const
{
	assert(image && view && "The image and its view must be created before updating the descriptor image info.");

	descriptorImageInfo.sampler = SamplerManager::getSampler(samplerType);
	descriptorImageInfo.imageView = view;
	descriptorImageInfo.imageLayout = layout;
}

void mtd::Image::transitionImageLayout
(
	vk::CommandBuffer commandBuffer, vk::ImageLayout newLayout,
	vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage
) const
{
	assert(image && "The image must be created before transitioning it.");

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
	assert(image && "The image must be created before copying to it.");

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

	transitionImageLayout(commandBuffer, vk::ImageLayout::eTransferDstOptimal);
	commandBuffer.copyBufferToImage
	(
		srcBuffer, image, vk::ImageLayout::eTransferDstOptimal, bufferImageCopy
	);
	transitionImageLayout
	(
		commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eAllGraphics
	);

	commandHandler.endSingleTimeCommand(commandBuffer);
}

void mtd::Image::createImage()
{
	assert(!image && "The Vulkan image has already been created.");
	assert(dimensions.x != 0U && dimensions.y != 0U && "The image must have a valid size to be created.");

	vk::ImageCreateInfo imageCreateInfo{};
	imageCreateInfo.flags = createFlags;
	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent = vk::Extent3D{dimensions.x, dimensions.y, 1U};
	imageCreateInfo.mipLevels = 1U;
	imageCreateInfo.arrayLayers = 1U;
	imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.usage = usageFlags;
	imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	imageCreateInfo.queueFamilyIndexCount = 0U;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = layout;

	vk::Result result = mtdDevice.getDevice().createImage(&imageCreateInfo, nullptr, &image);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create image. Vulkan result: %d", result);
}

void mtd::Image::createMemory()
{
	assert(image && "The image must be created before the image memory.");
	assert(!imageMemory && "The Vulkan image memory has already been created.");

	vk::MemoryRequirements requirements = mtdDevice.getDevice().getImageMemoryRequirements(image);

	vk::MemoryAllocateInfo allocationInfo{};
	allocationInfo.allocationSize = requirements.size;
	allocationInfo.memoryTypeIndex = Memory::findMemoryTypeIndex
	(
		mtdDevice.getPhysicalDevice(),
		requirements.memoryTypeBits,
		memoryProperties
	);

	vk::Result result = mtdDevice.getDevice().allocateMemory(&allocationInfo, nullptr, &imageMemory);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to allocate memory for image. Vulkan result: %d", result);
		return;
	}

	mtdDevice.getDevice().bindImageMemory(image, imageMemory, 0UL);
}

void mtd::Image::createView()
{
	assert(image && "The image must be created before the image view.");
	assert(!view && "The Vulkan image view has already been created.");

	vk::ComponentMapping componentMapping{};
	componentMapping.r = vk::ComponentSwizzle::eIdentity;
	componentMapping.g = vk::ComponentSwizzle::eIdentity;
	componentMapping.b = vk::ComponentSwizzle::eIdentity;
	componentMapping.a = vk::ComponentSwizzle::eIdentity;

	vk::ImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = aspectFlags;
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

	vk::Result result = mtdDevice.getDevice().createImageView(&imageViewCreateInfo, nullptr, &view);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create image view. Vulkan result: %d", result);
}
