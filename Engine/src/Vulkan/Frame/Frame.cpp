#include <pch.hpp>
#include "Frame.hpp"

#include "../../Utils/Logger.hpp"
#include "../Command/Synchronization.hpp"
#include "../Image/Image.hpp"

mtd::Frame::Frame
(
	const Device& device,
	const FrameDimensions& frameDimensions,
	vk::Image image,
	vk::Format format,
	uint32_t frameIndex
) : device{device.getDevice()},
	frameDimensions{frameDimensions},
	image{image},
	commandHandler{device},
	frameIndex{frameIndex}
{
	Image::createImageView
	(
		device.getDevice(),
		image,
		format,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::e2D,
		imageView
	);

	Synchronization::createFence(this->device, synchronizationBundle.inFlightFence);
	Synchronization::createSemaphore(this->device, synchronizationBundle.imageAvailable);
	Synchronization::createSemaphore(this->device, synchronizationBundle.renderFinished);

	createDepthResources(device);

	LOG_VERBOSE("Created frame number %d.", frameIndex);
}

mtd::Frame::~Frame()
{
	device.destroyImage(depthBuffer.image);
	device.freeMemory(depthBuffer.imageMemory);
	device.destroyImageView(depthBuffer.imageView);

	device.destroySemaphore(synchronizationBundle.renderFinished);
	device.destroySemaphore(synchronizationBundle.imageAvailable);
	device.destroyFence(synchronizationBundle.inFlightFence);

	device.destroyFramebuffer(framebuffer);
	device.destroyImageView(imageView);
}

mtd::Frame::Frame(Frame&& other) noexcept
	: device{other.device},
	frameIndex{other.frameIndex},
	frameDimensions{other.frameDimensions},
	image{std::move(other.image)},
	imageView{std::move(other.imageView)},
	framebuffer{std::move(other.framebuffer)},
	depthBuffer{std::move(other.depthBuffer)},
	commandHandler{std::move(other.commandHandler)},
	synchronizationBundle{std::move(other.synchronizationBundle)}
{
	other.image = nullptr;
	other.imageView = nullptr;
	other.framebuffer = nullptr;
	other.depthBuffer.image = nullptr;
	other.depthBuffer.imageView = nullptr;
	other.depthBuffer.imageMemory = nullptr;
	other.synchronizationBundle.inFlightFence = nullptr;
	other.synchronizationBundle.imageAvailable = nullptr;
	other.synchronizationBundle.renderFinished = nullptr;
}

// Adds frame data to the draw info
void mtd::Frame::fetchFrameDrawData(DrawInfo& drawInfo) const
{
	drawInfo.framebuffer = &framebuffer;
	drawInfo.syncBundle = &synchronizationBundle;
}

// Set up framebuffer
void mtd::Frame::createFramebuffer(const vk::RenderPass& renderPass)
{
	std::vector<vk::ImageView> attachments{imageView, depthBuffer.imageView};

	vk::FramebufferCreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.flags = vk::FramebufferCreateFlags();
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferCreateInfo.pAttachments = attachments.data();
	framebufferCreateInfo.width = frameDimensions.width;
	framebufferCreateInfo.height = frameDimensions.height;
	framebufferCreateInfo.layers = 1;

	vk::Result result = device.createFramebuffer(&framebufferCreateInfo, nullptr, &framebuffer);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create framebuffer. Vulkan result: %d", result);
		return;
	}

	LOG_VERBOSE("Created framebuffer.");
}

// Creates depth buffer data
void mtd::Frame::createDepthResources(const Device& device)
{
	depthBuffer.format = findSupportedFormat
	(
		device.getPhysicalDevice(),
		{vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint},
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);

	Image::CreateImageBundle imageBundle{device.getDevice()};
	imageBundle.tiling = vk::ImageTiling::eOptimal;
	imageBundle.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	imageBundle.format = depthBuffer.format;
	imageBundle.imageFlags = vk::ImageCreateFlags();
	imageBundle.dimensions = frameDimensions;

	Image::createImage(imageBundle, depthBuffer.image);
	Image::createImageMemory
	(
		device, depthBuffer.image, vk::MemoryPropertyFlagBits::eDeviceLocal, depthBuffer.imageMemory
	);
	Image::createImageView
	(
		device.getDevice(),
		depthBuffer.image,
		depthBuffer.format,
		vk::ImageAspectFlagBits::eDepth,
		vk::ImageViewType::e2D,
		depthBuffer.imageView
	);
}

// Selects an image format with the specified features
vk::Format mtd::Frame::findSupportedFormat
(
	const vk::PhysicalDevice& physicalDevice,
	const std::vector<vk::Format>& candidates,
	vk::ImageTiling tiling,
	vk::FormatFeatureFlags features
)
{
	for(vk::Format candidate: candidates)
	{
		vk::FormatProperties properties = physicalDevice.getFormatProperties(candidate);
		if
		(
			(tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features) ||
			(tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features)
		) return candidate;
	}

	LOG_ERROR("Failed to find suitable Vulkan format.");
	return vk::Format::eUndefined;
}
