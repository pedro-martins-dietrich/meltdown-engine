#include "Frame.hpp"

#include "../../Utils/Logger.hpp"

mtd::Frame::Frame
(
	const vk::Device& device,
	const FrameDimensions& frameDimensions,
	vk::Image image,
	vk::Format format
) : device{device}, frameDimensions{frameDimensions}, image{image}
{
	createImageView(format);

	LOG_VERBOSE("Created a frame.");
}

mtd::Frame::~Frame()
{
	device.destroyImageView(imageView);
}

mtd::Frame::Frame(Frame&& frame) noexcept
	: device{frame.device},
	frameDimensions{std::move(frame.frameDimensions)},
	image{std::move(frame.image)},
	imageView{std::move(frame.imageView)}
{
	frame.image = nullptr;
	frame.imageView = nullptr;
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
		LOG_ERROR("Failed to create image view for frame.");
}
