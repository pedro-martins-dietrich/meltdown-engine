#include <pch.hpp>
#include "Frame.hpp"

#include "../../Utils/Logger.hpp"
#include "../Command/Synchronization.hpp"

mtd::Frame::Frame
(
	const Device& mtdDevice,
	const FrameDimensions& frameDimensions,
	vk::Image image,
	vk::Format format,
	uint32_t frameIndex
) : device{mtdDevice.getDevice()},
	framebuffer{nullptr},
	colorBuffer{mtdDevice.getDevice()},
	depthBuffer{mtdDevice.getDevice()},
	frameIndex{frameIndex},
	frameDimensions{frameDimensions},
	commandHandler{mtdDevice}
{
	colorBuffer.setVulkanImage(image, format, frameDimensions);
	colorBuffer.createImageView(vk::ImageAspectFlagBits::eColor, vk::ImageViewType::e2D);

	Synchronization::createFence(device, synchronizationBundle.inFlightFence);
	Synchronization::createSemaphore(device, synchronizationBundle.imageAvailable);
	Synchronization::createSemaphore(device, synchronizationBundle.renderFinished);

	createDepthResources(mtdDevice);

	LOG_VERBOSE("Created frame number %d.", frameIndex);
}

mtd::Frame::~Frame()
{
	device.destroySemaphore(synchronizationBundle.renderFinished);
	device.destroySemaphore(synchronizationBundle.imageAvailable);
	device.destroyFence(synchronizationBundle.inFlightFence);

	device.destroyFramebuffer(framebuffer);
	colorBuffer.setVulkanImage(nullptr, vk::Format::eUndefined, {0U, 0U});
}

mtd::Frame::Frame(Frame&& other) noexcept
	: device{other.device},
	framebuffer{std::move(other.framebuffer)},
	colorBuffer{std::move(other.colorBuffer)},
	depthBuffer{std::move(other.depthBuffer)},
	frameIndex{other.frameIndex},
	frameDimensions{other.frameDimensions},
	commandHandler{std::move(other.commandHandler)},
	synchronizationBundle{std::move(other.synchronizationBundle)}
{
	other.framebuffer = nullptr;
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
	std::vector<vk::ImageView> attachments{colorBuffer.getImageView(), depthBuffer.getImageView()};

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
void mtd::Frame::createDepthResources(const Device& mtdDevice)
{
	vk::Format depthBufferFormat = findSupportedFormat
	(
		mtdDevice.getPhysicalDevice(),
		{vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint},
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);

	depthBuffer.createImage
	(
		frameDimensions, depthBufferFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment
	);
	depthBuffer.createImageMemory(mtdDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
	depthBuffer.createImageView(vk::ImageAspectFlagBits::eDepth, vk::ImageViewType::e2D);
}

// Selects an image format with the specified features
vk::Format mtd::Frame::findSupportedFormat
(
	const vk::PhysicalDevice& physicalDevice,
	const std::vector<vk::Format>& candidates,
	vk::ImageTiling tiling,
	vk::FormatFeatureFlags features
) const
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
