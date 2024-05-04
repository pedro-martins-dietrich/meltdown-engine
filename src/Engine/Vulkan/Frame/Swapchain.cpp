#include "Swapchain.hpp"

#include "../../Utils/Logger.hpp"

mtd::Swapchain::Swapchain
(
	const Device& device,
	const FrameDimensions& frameDimensions,
	const vk::SurfaceKHR& surface
) : device{device.getDevice()}, frameCount{3}
{
	configureDefaultSettings();
	getSupportedDetails(device.getPhysicalDevice(), surface);
	selectImageCount();
	createSwapchain(device, frameDimensions, surface);
}

mtd::Swapchain::~Swapchain()
{
	destroy();
}

// Create framebuffers for each frame
void mtd::Swapchain::createFramebuffers(const vk::RenderPass& renderPass)
{
	for(Frame& frame: frames)
	{
		frame.createFramebuffer(renderPass);
	}
}

// Recreates swapchain to handle resizes
void mtd::Swapchain::recreate
(
	const Device& device,
	const FrameDimensions& frameDimensions,
	const vk::SurfaceKHR& surface
)
{
	destroy();
	getSupportedDetails(device.getPhysicalDevice(), surface);
	createSwapchain(device, frameDimensions, surface);
}

// Sets up default swapchain settings
void mtd::Swapchain::configureDefaultSettings()
{
	settings.colorFormat = vk::Format::eB8G8R8A8Unorm;
	settings.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	settings.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	settings.presentMode = vk::PresentModeKHR::eMailbox;
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
	checkSurfaceFormat();

	uint32_t distinctQueueFamilyIndices =
		static_cast<uint32_t>(device.getQueueFamilies().getUniqueIndices().size());
	vk::SharingMode selectedSharingMode = distinctQueueFamilyIndices
		? vk::SharingMode::eExclusive
		: vk::SharingMode::eConcurrent;

	selectExtent(frameDimensions);
	checkPresentMode();

	vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.flags = vk::SwapchainCreateFlagsKHR();
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = frameCount;
	swapchainCreateInfo.imageFormat = settings.colorFormat;
	swapchainCreateInfo.imageColorSpace = settings.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1U;
	swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	swapchainCreateInfo.imageSharingMode = selectedSharingMode;
	swapchainCreateInfo.queueFamilyIndexCount = distinctQueueFamilyIndices;
	swapchainCreateInfo.pQueueFamilyIndices = device.getQueueFamilies().getUniqueIndices().data();
	swapchainCreateInfo.preTransform = supportedDetails.capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = settings.compositeAlpha;
	swapchainCreateInfo.presentMode = settings.presentMode;
	swapchainCreateInfo.clipped = vk::True;
	swapchainCreateInfo.oldSwapchain = nullptr;

	vk::Result result = device.getDevice().createSwapchainKHR(&swapchainCreateInfo, nullptr, &swapchain);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create swapchain.");
		return;
	}

	setSwapchainFrames(device, frameDimensions);

	LOG_INFO("Created swapchain.\n");
}

// Ensures the swapchain uses a valid surface format
void mtd::Swapchain::checkSurfaceFormat()
{
	for(vk::SurfaceFormatKHR supportedFormat: supportedDetails.formats)
	{
		if(supportedFormat.format == settings.colorFormat &&
			supportedFormat.colorSpace == settings.colorSpace)
		{
			return;
		}
	}

	LOG_WARNING("Could not find a surface format with the desired properties.");

	settings.colorFormat = supportedDetails.formats[0].format;
	settings.colorSpace = supportedDetails.formats[0].colorSpace;
}

// Sets how many frames will be stored in the buffer
void mtd::Swapchain::selectImageCount()
{
	frameCount = std::clamp
	(
		frameCount,
		supportedDetails.capabilities.minImageCount,
		supportedDetails.capabilities.maxImageCount
	);
}

// Sets the frame dimensions to be used in the swapchain
void mtd::Swapchain::selectExtent(const FrameDimensions& frameDimensions)
{
	if(supportedDetails.capabilities.currentExtent.width != UINT32_MAX)
	{
		extent = supportedDetails.capabilities.currentExtent;
		return;
	}

	extent.width = std::clamp
	(
		frameDimensions.width,
		supportedDetails.capabilities.minImageExtent.width,
		supportedDetails.capabilities.maxImageExtent.width
	);
	extent.height = std::clamp
	(
		frameDimensions.height,
		supportedDetails.capabilities.minImageExtent.height,
		supportedDetails.capabilities.maxImageExtent.height
	);
}

// Ensures the present mode to be used is valid
void mtd::Swapchain::checkPresentMode()
{
	for(vk::PresentModeKHR supportedPresentMode: supportedDetails.presentModes)
	{
		if(settings.presentMode == supportedPresentMode)
		{
			LOG_VERBOSE("Swapchain present mode %d selected.", supportedPresentMode);
			return;
		}
	}

	LOG_WARNING
	(
		"Desired swapchain present mode (%d) not supported. Using FIFO (%d).",
		settings.presentMode,
		vk::PresentModeKHR::eFifo
	);
	settings.presentMode = vk::PresentModeKHR::eFifo;
}

// Creates all the swapchain frames
void mtd::Swapchain::setSwapchainFrames
(
	const Device& device, const FrameDimensions& frameDimensions
)
{
	std::vector<vk::Image> images = this->device.getSwapchainImagesKHR(swapchain);

	frames.reserve(frameCount);
	for(uint32_t i = 0; i < images.size(); i++)
	{
		frames.emplace_back(device, frameDimensions, images[i], settings.colorFormat, i);
	}
}

// Destroys the swapchain
void mtd::Swapchain::destroy()
{
	frames.clear();
	device.destroySwapchainKHR(swapchain);
}
