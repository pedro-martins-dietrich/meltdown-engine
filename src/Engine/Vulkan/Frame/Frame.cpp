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
	device.destroyImage(depthBuffer);
	device.freeMemory(depthBufferMemory);
	device.destroyImageView(depthBufferView);

	device.destroySemaphore(synchronizationBundle.renderFinished);
	device.destroySemaphore(synchronizationBundle.imageAvailable);
	device.destroyFence(synchronizationBundle.inFlightFence);

	device.destroyFramebuffer(framebuffer);
	device.destroyImageView(imageView);
}

mtd::Frame::Frame(Frame&& frame) noexcept
	: device{frame.device},
	frameIndex{frame.frameIndex},
	frameDimensions{frame.frameDimensions},
	image{std::move(frame.image)},
	imageView{std::move(frame.imageView)},
	framebuffer{std::move(frame.framebuffer)},
	depthBuffer{std::move(frame.depthBuffer)},
	depthBufferView{std::move(frame.depthBufferView)},
	depthBufferMemory{std::move(frame.depthBufferMemory)},
	depthBufferFormat{frame.depthBufferFormat},
	commandHandler{std::move(frame.commandHandler)},
	synchronizationBundle{std::move(frame.synchronizationBundle)}
{
	frame.image = nullptr;
	frame.imageView = nullptr;
	frame.framebuffer = nullptr;
	frame.depthBuffer = nullptr;
	frame.depthBufferView = nullptr;
	frame.depthBufferMemory = nullptr;
	frame.synchronizationBundle.inFlightFence = nullptr;
	frame.synchronizationBundle.imageAvailable = nullptr;
	frame.synchronizationBundle.renderFinished = nullptr;
}

// Set up framebuffer
void mtd::Frame::createFramebuffer(const vk::RenderPass& renderPass)
{
	std::vector<vk::ImageView> attachments{imageView, depthBufferView};

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

// Draws frame to screen
void mtd::Frame::drawFrame(DrawInfo& drawInfo) const
{
	drawInfo.framebuffer = &framebuffer;
	drawInfo.syncBundle = &synchronizationBundle;
	drawInfo.frameIndex = frameIndex;

	commandHandler.draw(drawInfo);
}

// Creates depth buffer data
void mtd::Frame::createDepthResources(const Device& device)
{
	depthBufferFormat = findSupportedFormat
	(
		device.getPhysicalDevice(),
		{vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint},
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);

	Image::CreateImageBundle imageBundle{device.getDevice()};
	imageBundle.tiling = vk::ImageTiling::eOptimal;
	imageBundle.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	imageBundle.format = depthBufferFormat;
	imageBundle.imageFlags = vk::ImageCreateFlags();
	imageBundle.dimensions = frameDimensions;

	Image::createImage(imageBundle, depthBuffer);
	Image::createImageMemory
	(
		device, depthBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, depthBufferMemory
	);
	Image::createImageView
	(
		device.getDevice(),
		depthBuffer,
		depthBufferFormat,
		vk::ImageAspectFlagBits::eDepth,
		vk::ImageViewType::e2D,
		depthBufferView
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
			(tiling == vk::ImageTiling::eLinear &&
				(properties.linearTilingFeatures & features) == features)
			||
			(tiling == vk::ImageTiling::eOptimal &&
				(properties.optimalTilingFeatures & features) == features)
		)
		{
			return candidate;
		}
	}

	LOG_ERROR("Failed to find suitable Vulkan format.");
	return vk::Format::eUndefined;
}
