#include "Swapchain.hpp"

#include "../../Utils/Logger.hpp"

mtd::Swapchain::Swapchain
(
	const Device& device,
	const FrameDimensions& frameDimensions,
	const vk::SurfaceKHR& surface
) : device{device.getDevice()}, frameCount{3}
{
	getSupportedDetails(device.getPhysicalDevice(), surface);
	createSwapchain(device, frameDimensions, surface);
}

mtd::Swapchain::~Swapchain()
{
	destroy();
}

// Retrieves swapchain features supported by the physical device
void mtd::Swapchain::getSupportedDetails
(
	const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface
)
{
	supportedDetails.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	supportedDetails.formats = physicalDevice.getSurfaceFormatsKHR(surface);
	supportedDetails.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
}

// Creates the swapchain
void mtd::Swapchain::createSwapchain
(
	const Device& device,
	const FrameDimensions& frameDimensions,
	const vk::SurfaceKHR& surface
)
{
	vk::SurfaceFormatKHR selectedFormat =
		selectFormat(vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear);

	uint32_t distinctQueueFamilyIndices =
		static_cast<uint32_t>(device.getQueueFamilies().getUniqueIndices().size());
	vk::SharingMode selectedSharingMode = distinctQueueFamilyIndices
		? vk::SharingMode::eExclusive
		: vk::SharingMode::eConcurrent;

	vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.flags = vk::SwapchainCreateFlagsKHR();
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = selectImageCount(frameCount);
	swapchainCreateInfo.imageFormat = selectedFormat.format;
	swapchainCreateInfo.imageColorSpace = selectedFormat.colorSpace;
	swapchainCreateInfo.imageExtent = selectExtent(frameDimensions);
	swapchainCreateInfo.imageArrayLayers = 1U;
	swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	swapchainCreateInfo.imageSharingMode = selectedSharingMode;
	swapchainCreateInfo.queueFamilyIndexCount = distinctQueueFamilyIndices;
	swapchainCreateInfo.pQueueFamilyIndices = device.getQueueFamilies().getUniqueIndices().data();
	swapchainCreateInfo.preTransform = supportedDetails.capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapchainCreateInfo.presentMode = selectPresentMode(vk::PresentModeKHR::eMailbox);
	swapchainCreateInfo.clipped = vk::True;
	swapchainCreateInfo.oldSwapchain = nullptr;

	vk::Result result = device.getDevice().createSwapchainKHR(&swapchainCreateInfo, nullptr, &swapchain);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create swapchain.");
		return;
	}

	setSwapchainFrames(frameDimensions, selectedFormat.format);

	LOG_INFO("Created swapchain.\n");
}

// Selects the image format to be used
vk::SurfaceFormatKHR mtd::Swapchain::selectFormat
(
	vk::Format desiredFormat, vk::ColorSpaceKHR desiredColorSpace
) const
{
	for(vk::SurfaceFormatKHR supportedFormat: supportedDetails.formats)
	{
		if(supportedFormat.format == desiredFormat &&
			supportedFormat.colorSpace == desiredColorSpace)
		{
			return supportedFormat;
		}
	}

	LOG_WARNING("Could not find a surface format with the desired properties.");
	return supportedDetails.formats[0];
}

// Sets how many frames will be stored in the buffer
uint32_t mtd::Swapchain::selectImageCount(uint32_t desiredImageCount) const
{
	return std::clamp
	(
		desiredImageCount,
		supportedDetails.capabilities.minImageCount,
		supportedDetails.capabilities.maxImageCount
	);
}

// Sets the frame dimensions to be used in the swapchain
vk::Extent2D mtd::Swapchain::selectExtent(const FrameDimensions& frameDimensions) const
{
	if(supportedDetails.capabilities.currentExtent.width != UINT32_MAX)
	{
		return supportedDetails.capabilities.currentExtent;
	}

	return vk::Extent2D
	{
		std::clamp
		(
			frameDimensions.width,
			supportedDetails.capabilities.minImageExtent.width,
			supportedDetails.capabilities.maxImageExtent.width
		),
		std::clamp
		(
			frameDimensions.height,
			supportedDetails.capabilities.minImageExtent.height,
			supportedDetails.capabilities.maxImageExtent.height
		)
	};
}

// Sets the present mode to be used
vk::PresentModeKHR mtd::Swapchain::selectPresentMode(vk::PresentModeKHR desiredPresentMode) const
{
	for(vk::PresentModeKHR supportedPresentMode: supportedDetails.presentModes)
	{
		if(desiredPresentMode == supportedPresentMode)
		{
			LOG_VERBOSE("Swapchain present mode %d selected.", supportedPresentMode);
			return supportedPresentMode;
		}
	}

	LOG_WARNING
	(
		"Desired swapchain present mode (%d) not supported. Using FIFO (%d).",
		desiredPresentMode,
		vk::PresentModeKHR::eFifo
	);
	return vk::PresentModeKHR::eFifo;
}

// Creates all the swapchain frames
void mtd::Swapchain::setSwapchainFrames
(
	const FrameDimensions& frameDimensions, vk::Format format
)
{
	std::vector<vk::Image> images = device.getSwapchainImagesKHR(swapchain);

	frames.reserve(frameCount);
	for(vk::Image& image: images)
	{
		frames.emplace_back(device, frameDimensions, image, format);
	}
}

// Destroys the swapchain
void mtd::Swapchain::destroy()
{
	frames.clear();
	device.destroySwapchainKHR(swapchain);
}
