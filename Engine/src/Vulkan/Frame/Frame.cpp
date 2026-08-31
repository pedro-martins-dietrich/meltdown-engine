#include <pch.hpp>
#include "Frame.hpp"

#include "../../Utils/Logger.hpp"
#include "../Command/Synchronization.hpp"

mtd::Frame::Frame
(
	const Device& mtdDevice, UIntVec2 frameDimensions, vk::Image colorBufferImage, vk::Format colorBufferFormat
)
	: device{mtdDevice.getDevice()}, framebuffer{nullptr}, dimensions{frameDimensions},
	colorBuffer{colorBufferImage}, colorBufferView{nullptr}, depthBuffer{mtdDevice}, commandHandler{mtdDevice}
{
	Synchronization::createFence(device, synchronizationBundle.inFlightFence);
	Synchronization::createSemaphore(device, synchronizationBundle.imageAvailable);
	Synchronization::createSemaphore(device, synchronizationBundle.renderFinished);
	Synchronization::createSemaphore(device, synchronizationBundle.screenshotCopy);

	createColorBufferView(colorBufferFormat);
	createDepthResources(mtdDevice);
}

mtd::Frame::~Frame()
{
	device.destroySemaphore(synchronizationBundle.screenshotCopy);
	device.destroySemaphore(synchronizationBundle.renderFinished);
	device.destroySemaphore(synchronizationBundle.imageAvailable);
	device.destroyFence(synchronizationBundle.inFlightFence);

	device.destroyFramebuffer(framebuffer);
	device.destroyImageView(colorBufferView);
}

mtd::Frame::Frame(Frame&& other) noexcept
	: device{other.device},
	framebuffer{std::move(other.framebuffer)},
	dimensions{other.dimensions},
	colorBuffer{std::move(other.colorBuffer)},
	colorBufferView{std::move(other.colorBufferView)},
	depthBuffer{std::move(other.depthBuffer)},
	commandHandler{std::move(other.commandHandler)},
	synchronizationBundle{std::move(other.synchronizationBundle)}
{
	other.framebuffer = nullptr;
	other.synchronizationBundle.inFlightFence = nullptr;
	other.synchronizationBundle.imageAvailable = nullptr;
	other.synchronizationBundle.renderFinished = nullptr;
	other.synchronizationBundle.screenshotCopy = nullptr;
}

void mtd::Frame::createFramebuffer(const vk::RenderPass& renderPass)
{
	std::vector<vk::ImageView> attachments{colorBufferView, depthBuffer.getView()};

	vk::FramebufferCreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.flags = vk::FramebufferCreateFlags();
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferCreateInfo.pAttachments = attachments.data();
	framebufferCreateInfo.width = dimensions.x;
	framebufferCreateInfo.height = dimensions.y;
	framebufferCreateInfo.layers = 1U;

	vk::Result result = device.createFramebuffer(&framebufferCreateInfo, nullptr, &framebuffer);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create framebuffer. Vulkan result: %d", result);
		return;
	}

	LOG_VERBOSE("Created framebuffer.");
}

void mtd::Frame::createColorBufferView(vk::Format colorBufferFormat)
{
	vk::ComponentMapping componentMapping{};
	componentMapping.r = vk::ComponentSwizzle::eIdentity;
	componentMapping.g = vk::ComponentSwizzle::eIdentity;
	componentMapping.b = vk::ComponentSwizzle::eIdentity;
	componentMapping.a = vk::ComponentSwizzle::eIdentity;

	vk::ImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	subresourceRange.baseMipLevel = 0U;
	subresourceRange.levelCount = 1U;
	subresourceRange.baseArrayLayer = 0U;
	subresourceRange.layerCount = 1U;

	vk::ImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.flags = vk::ImageViewCreateFlags();
	imageViewCreateInfo.image = colorBuffer;
	imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
	imageViewCreateInfo.format = colorBufferFormat;
	imageViewCreateInfo.components = componentMapping;
	imageViewCreateInfo.subresourceRange = subresourceRange;

	vk::Result result = device.createImageView(&imageViewCreateInfo, nullptr, &colorBufferView);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create frame's color buffer image view. Vulkan result: %d", result);
}

void mtd::Frame::createDepthResources(const Device& mtdDevice)
{
	vk::Format depthBufferFormat = findSupportedFormat
	(
		mtdDevice.getPhysicalDevice(),
		{vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint},
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);

	depthBuffer.create
	(
		dimensions,
		depthBufferFormat,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageAspectFlagBits::eDepth,
		vk::ImageViewType::e2D,
		SamplerType::Linear
	);
}

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
			(tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features)
			|| (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features)
		) return candidate;
	}

	LOG_ERROR("Failed to find suitable Vulkan format.");
	return vk::Format::eUndefined;
}
