#include "Frame.hpp"

#include "../../Utils/Logger.hpp"

mtd::Frame::Frame
(
	const Device& device,
	const FrameDimensions& frameDimensions,
	vk::Image image,
	vk::Format format
) : device{device.getDevice()},
	frameDimensions{frameDimensions},
	image{image},
	commandHandler{device}
{
	createImageView(format);

	LOG_VERBOSE("Created a frame.");
}

mtd::Frame::~Frame()
{
	device.destroyFramebuffer(framebuffer);
	device.destroyImageView(imageView);
}

mtd::Frame::Frame(Frame&& frame) noexcept
	: device{frame.device},
	frameDimensions{std::move(frame.frameDimensions)},
	image{std::move(frame.image)},
	imageView{std::move(frame.imageView)},
	commandHandler{std::move(frame.commandHandler)}
{
	frame.image = nullptr;
	frame.imageView = nullptr;
}

// Set up framebuffer
void mtd::Frame::createFramebuffer(const vk::RenderPass& renderPass)
{
	vk::FramebufferCreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.flags = vk::FramebufferCreateFlags();
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = 1;
	framebufferCreateInfo.pAttachments = &imageView;
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

// Create frame image view
void mtd::Frame::createImageView(vk::Format format)
{
	vk::ComponentMapping componentMapping{};
	componentMapping.r = vk::ComponentSwizzle::eIdentity;
	componentMapping.g = vk::ComponentSwizzle::eIdentity;
	componentMapping.b = vk::ComponentSwizzle::eIdentity;
	componentMapping.a = vk::ComponentSwizzle::eIdentity;

	vk::ImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	vk::ImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.flags = vk::ImageViewCreateFlags();
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components = componentMapping;
	imageViewCreateInfo.subresourceRange = subresourceRange;

	vk::Result result = device.createImageView(&imageViewCreateInfo, nullptr, &imageView);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create image view for frame. Vulkan result: %d", result);
}
