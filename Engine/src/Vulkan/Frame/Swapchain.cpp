#include <pch.hpp>
#include "Swapchain.hpp"

#include "../../Utils/Logger.hpp"

mtd::Swapchain::Swapchain
(
	const Device& device,
	const FrameDimensions& frameDimensions,
	const vk::SurfaceKHR& surface
) : device{device.getDevice()}
{
	configureDefaultSettings();
	getSupportedDetails(device.getPhysicalDevice(), surface);
	checkImageCount();
	createSwapchain(device, frameDimensions, surface);
	createRenderPass();
}

mtd::Swapchain::~Swapchain()
{
	destroy();
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
	checkImageCount();
	createSwapchain(device, frameDimensions, surface);
	createRenderPass();
}

// Enables or disables V-Sync
bool mtd::Swapchain::setVSync(bool enableVSync)
{
	if(!enableVSync)
	{
		if(settings.presentMode == vk::PresentModeKHR::eFifo) return false;

		settings.presentMode = vk::PresentModeKHR::eFifo;
		return true;
	}

	if
	(
		settings.presentMode == vk::PresentModeKHR::eMailbox ||
		settings.presentMode == vk::PresentModeKHR::eImmediate
	) return false;

	if(isPresentModeAvailable(vk::PresentModeKHR::eMailbox))
	{
		settings.presentMode = vk::PresentModeKHR::eMailbox;
		return true;
	}
	else if(isPresentModeAvailable(vk::PresentModeKHR::eImmediate))
	{
		settings.presentMode = vk::PresentModeKHR::eImmediate;
		return true;
	}

	LOG_WARNING("V-Sync is not supported on this device.");
	return false;
}

// Sets up default swapchain settings
void mtd::Swapchain::configureDefaultSettings()
{
	settings.frameCount = 3;
	settings.colorFormat = vk::Format::eB8G8R8A8Unorm;
	settings.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	settings.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	settings.presentMode = vk::PresentModeKHR::eFifo;
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
	swapchainCreateInfo.minImageCount = settings.frameCount;
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

	vk::Result result =
		device.getDevice().createSwapchainKHR(&swapchainCreateInfo, nullptr, &swapchain);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create swapchain.");
		return;
	}

	setSwapchainFrames(device, frameDimensions);

	LOG_INFO("Created swapchain.\n");
}

// Creates pipeline render pass
void mtd::Swapchain::createRenderPass()
{
	vk::AttachmentDescription colorAttachmentDescription{};
	colorAttachmentDescription.flags = vk::AttachmentDescriptionFlags();
	colorAttachmentDescription.format = settings.colorFormat;
	colorAttachmentDescription.samples = vk::SampleCountFlagBits::e1;
	colorAttachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachmentDescription.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference colorAttachmentReference{};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentDescription depthAttachmentDescription{};
	depthAttachmentDescription.flags = vk::AttachmentDescriptionFlags();
	depthAttachmentDescription.format = frames[0].getDepthFormat();
	depthAttachmentDescription.samples = vk::SampleCountFlagBits::e1;
	depthAttachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
	depthAttachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachmentDescription.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference depthAttachmentReference{};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	std::vector<vk::AttachmentDescription> attachmentDescriptions
	{
		colorAttachmentDescription, depthAttachmentDescription
	};

	vk::SubpassDescription subpassDescription{};
	subpassDescription.flags = vk::SubpassDescriptionFlags();
	subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	subpassDescription.pResolveAttachments = nullptr;
	subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;

	vk::RenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.flags = vk::RenderPassCreateFlags();
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 0;
	renderPassCreateInfo.pDependencies = nullptr;

	vk::Result result = device.createRenderPass(&renderPassCreateInfo, nullptr, &renderPass);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create render pass. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created render pass.");
	createFramebuffers();
}

// Create framebuffers for each frame
void mtd::Swapchain::createFramebuffers()
{
	for(Frame& frame: frames)
		frame.createFramebuffer(renderPass);
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

// Ensures a valid amount of frames to be stored in the buffer
void mtd::Swapchain::checkImageCount()
{
	if(supportedDetails.capabilities.maxImageCount == 0)
	{
		settings.frameCount =
			std::max(settings.frameCount, supportedDetails.capabilities.minImageCount);
		return;
	}

	settings.frameCount = std::clamp
	(
		settings.frameCount,
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
	if(isPresentModeAvailable(settings.presentMode))
	{
		LOG_VERBOSE("Swapchain present mode %d selected.", settings.presentMode);
		return;
	}

	LOG_WARNING
	(
		"Desired swapchain present mode (%d) not supported. Using FIFO (%d).",
		settings.presentMode,
		vk::PresentModeKHR::eFifo
	);
	settings.presentMode = vk::PresentModeKHR::eFifo;
}

// Verifies if the hardware supports the present mode
bool mtd::Swapchain::isPresentModeAvailable(vk::PresentModeKHR presentMode) const
{
	for(vk::PresentModeKHR supportedPresentMode: supportedDetails.presentModes)
		if(presentMode == supportedPresentMode) return true;
	return false;
}

// Creates all the swapchain frames
void mtd::Swapchain::setSwapchainFrames
(
	const Device& device, const FrameDimensions& frameDimensions
)
{
	std::vector<vk::Image> images = this->device.getSwapchainImagesKHR(swapchain);

	frames.reserve(settings.frameCount);
	LOG_INFO("Reserved %d frames.", settings.frameCount);
	for(uint32_t i = 0; i < images.size(); i++)
		frames.emplace_back(device, frameDimensions, images[i], settings.colorFormat, i);
}

// Destroys the swapchain
void mtd::Swapchain::destroy()
{
	device.destroyRenderPass(renderPass);
	frames.clear();
	device.destroySwapchainKHR(swapchain);
}
