#include <pch.hpp>
#include "Framebuffer.hpp"

#include "../Image/Image.hpp"
#include "../../Utils/Logger.hpp"

mtd::Framebuffer::Framebuffer
(
	const Device& mtdDevice, const FramebufferInfo& fbInfo, vk::Extent2D swapchainExtent
) : device{mtdDevice.getDevice()}, info{fbInfo}, windowResolutionDependant{false}
{
	createRenderPass();
	createSampler();
	createAttachments(mtdDevice, swapchainExtent);
	createFramebuffer();

	LOG_INFO("Created custom %dx%d framebuffer.\n", info.width, info.height);
}

mtd::Framebuffer::~Framebuffer()
{
	device.destroyFramebuffer(framebuffer);

	for(AttachmentData& attachmentData: attachmentsData)
	{
		device.destroyImageView(attachmentData.imageView);
		device.freeMemory(attachmentData.imageMemory);
		device.destroyImage(attachmentData.image);
	}

	device.destroySampler(sampler);
	device.destroyRenderPass(renderPass);
}

mtd::Framebuffer::Framebuffer(Framebuffer&& other) noexcept
	: device{other.device},
	info{std::move(other.info)},
	framebuffer{std::move(other.framebuffer)},
	renderPass{std::move(other.renderPass)},
	sampler{std::move(other.sampler)},
	attachmentsData{std::move(other.attachmentsData)},
	descriptorInfos{std::move(other.descriptorInfos)},
	windowResolutionDependant{other.windowResolutionDependant}
{
	other.framebuffer = nullptr;
	other.renderPass = nullptr;
	other.sampler = nullptr;
}

// Configures the specified attachment to be used as a descriptor
void mtd::Framebuffer::configureAttachmentAsDescriptor
(
	DescriptorSetHandler& descriptorSetHandler, uint32_t binding, uint32_t attachmentIndex
) const
{
	descriptorSetHandler.createImageDescriptorResources(0, binding, descriptorInfos[attachmentIndex]);
}

// Creates an image barrier to transition the attachment layout
void mtd::Framebuffer::transitionAttachmentLayout
(
	bool toShaderReadOnly,
	uint32_t attachmentIndex,
	vk::ImageMemoryBarrier& barrier,
	const vk::CommandBuffer& commandBuffer
) const
{
	assert(attachmentIndex < attachmentsData.size() && "Attachment index out of bounds.");
	const AttachmentData& attachmentData = attachmentsData[attachmentIndex];

	bool useDepth = static_cast<uint32_t>(info.framebufferAttachments) & 0x01U;

	vk::PipelineStageFlags pipelineSrcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::PipelineStageFlags pipelineDstStage = vk::PipelineStageFlagBits::eFragmentShader;

	if(useDepth && (attachmentIndex == (attachmentsData.size() - 1)))
	{
		if(toShaderReadOnly)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			barrier.oldLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			pipelineSrcStage =
				vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
		}
		else
		{
			barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			barrier.newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			pipelineDstStage =
				vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
		}
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
	}
	else
	{
		if(toShaderReadOnly)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
		}
		else
		{
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
			pipelineDstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		}
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	}

	if(toShaderReadOnly)
	{
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	}
	else
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		pipelineSrcStage = vk::PipelineStageFlagBits::eFragmentShader;
	}

	barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
	barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
	barrier.image = attachmentData.image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	commandBuffer.pipelineBarrier
	(
		pipelineSrcStage, pipelineDstStage,
		vk::DependencyFlags(),
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}

// Resizes the framebuffer resolution if needed
void mtd::Framebuffer::resize(const Device& mtdDevice, vk::Extent2D swapchainExtent)
{
	if(!windowResolutionDependant) return;

	device.destroyFramebuffer(framebuffer);
	for(AttachmentData& attachmentData: attachmentsData)
	{
		device.destroyImageView(attachmentData.imageView);
		device.freeMemory(attachmentData.imageMemory);
		device.destroyImage(attachmentData.image);
	}

	createAttachments(mtdDevice, swapchainExtent);
	createFramebuffer();
}

// Creates the Vulkan render pass for the framebuffer
void mtd::Framebuffer::createRenderPass()
{
	uint32_t colorAttachmentCount = 1U + (static_cast<uint32_t>(info.framebufferAttachments) >> 1);
	bool useDepth = static_cast<uint32_t>(info.framebufferAttachments) & 0x01U;
	uint32_t totalAttachmentCount = useDepth ? (colorAttachmentCount + 1) : colorAttachmentCount;

	std::vector<vk::AttachmentDescription> attachments(totalAttachmentCount);
	std::vector<vk::AttachmentReference> colorAttachmentReferences(colorAttachmentCount);

	for(uint32_t i = 0; i < colorAttachmentCount; i++)
	{
		attachments[i].flags = vk::AttachmentDescriptionFlags();
		attachments[i].format = vk::Format::eB8G8R8A8Unorm;
		attachments[i].samples = vk::SampleCountFlagBits::e1;
		attachments[i].loadOp = vk::AttachmentLoadOp::eClear;
		attachments[i].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[i].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[i].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[i].initialLayout = vk::ImageLayout::eUndefined;
		attachments[i].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

		colorAttachmentReferences[i].attachment = i;
		colorAttachmentReferences[i].layout = vk::ImageLayout::eColorAttachmentOptimal;
	}

	vk::AttachmentReference depthAttachmentReference{};
	if(useDepth)
	{
		attachments.back().flags = vk::AttachmentDescriptionFlags();
		attachments.back().format = vk::Format::eD32Sfloat;
		attachments.back().samples = vk::SampleCountFlagBits::e1;
		attachments.back().loadOp = vk::AttachmentLoadOp::eClear;
		attachments.back().storeOp = vk::AttachmentStoreOp::eStore;
		attachments.back().stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments.back().stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments.back().initialLayout = vk::ImageLayout::eUndefined;
		attachments.back().finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		depthAttachmentReference.attachment = colorAttachmentCount;
		depthAttachmentReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	}

	vk::SubpassDescription subpassDescription{};
	subpassDescription.flags = vk::SubpassDescriptionFlags();
	subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferences.size());
	subpassDescription.pColorAttachments = colorAttachmentReferences.data();
	subpassDescription.pResolveAttachments = nullptr;
	subpassDescription.pDepthStencilAttachment = useDepth ? &depthAttachmentReference : nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;

	vk::RenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.flags = vk::RenderPassCreateFlags();
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassCreateInfo.pAttachments = attachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 0;
	renderPassCreateInfo.pDependencies = nullptr;

	vk::Result result = device.createRenderPass(&renderPassCreateInfo, nullptr, &renderPass);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create framebuffer render pass. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created framebuffer render pass.");
}

// Creates the attachment objects
void mtd::Framebuffer::createAttachments(const Device& mtdDevice, vk::Extent2D swapchainExtent)
{
	if(info.windowResolutionRatio.x > 0.0f)
	{
		info.width = static_cast<uint32_t>(info.windowResolutionRatio.x * swapchainExtent.width);
		windowResolutionDependant = true;
	}
	if(info.windowResolutionRatio.y > 0.0f)
	{
		info.height = static_cast<uint32_t>(info.windowResolutionRatio.y * swapchainExtent.height);
		windowResolutionDependant = true;
	}

	uint32_t colorAttachmentCount = 1U + (static_cast<uint32_t>(info.framebufferAttachments) >> 1);
	bool useDepth = static_cast<uint32_t>(info.framebufferAttachments) & 0x01U;
	uint32_t totalAttachmentCount = useDepth ? (colorAttachmentCount + 1) : colorAttachmentCount;

	attachmentsData.resize(totalAttachmentCount);
	descriptorInfos.resize(totalAttachmentCount);

	for(uint32_t i = 0; i < colorAttachmentCount; i++)
	{
		attachmentsData[i].format = vk::Format::eB8G8R8A8Unorm;
		createAttachment
		(
			mtdDevice,
			attachmentsData[i],
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
			vk::ImageAspectFlagBits::eColor
		);
		descriptorInfos[i].sampler = sampler;
		descriptorInfos[i].imageView = attachmentsData[i].imageView;
		descriptorInfos[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	}

	if(useDepth)
	{
		attachmentsData.back().format = vk::Format::eD32Sfloat;
		createAttachment
		(
			mtdDevice,
			attachmentsData.back(),
			vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
			vk::ImageAspectFlagBits::eDepth
		);
		descriptorInfos.back().sampler = sampler;
		descriptorInfos.back().imageView = attachmentsData.back().imageView;
		descriptorInfos.back().imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	}
}

// Creates the data for a single attachment
void mtd::Framebuffer::createAttachment
(
	const Device& mtdDevice,
	AttachmentData& attachmentData,
	vk::ImageUsageFlags usage,
	vk::ImageAspectFlags aspect
) const
{
	Image::CreateImageBundle createImageBundle
	{
		device,
		vk::ImageTiling::eOptimal,
		usage,
		attachmentData.format,
		vk::ImageCreateFlags(),
		{info.width, info.height}
	};
	Image::createImage(createImageBundle, attachmentData.image);
	Image::createImageMemory
	(
		mtdDevice,
		attachmentData.image,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		attachmentData.imageMemory
	);
	Image::createImageView
	(
		device,
		attachmentData.image,
		attachmentData.format,
		aspect,
		vk::ImageViewType::e2D,
		attachmentData.imageView
	);
}

// Creates the Vulkan framebuffer object
void mtd::Framebuffer::createFramebuffer()
{
	std::vector<vk::ImageView> attachments(attachmentsData.size());
	for(uint32_t i = 0; i < attachmentsData.size(); i++)
		attachments[i] = attachmentsData[i].imageView;

	vk::FramebufferCreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.flags = vk::FramebufferCreateFlags();
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferCreateInfo.pAttachments = attachments.data();
	framebufferCreateInfo.width = info.width;
	framebufferCreateInfo.height = info.height;
	framebufferCreateInfo.layers = 1;

	vk::Result result = device.createFramebuffer(&framebufferCreateInfo, nullptr, &framebuffer);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create framebuffer. Vulkan result: %d", result);
		return;
	}

	LOG_VERBOSE("Created framebuffer.");
}

// Creates sampler to define how the attachments should be rendered
void mtd::Framebuffer::createSampler()
{
	vk::Filter filter = vk::Filter::eLinear;
	vk::SamplerMipmapMode mipmapMode = vk::SamplerMipmapMode::eLinear;
	switch(info.samplingFilter)
	{
		case TextureSamplingFilterType::Nearest:
			filter = vk::Filter::eNearest;
			mipmapMode = vk::SamplerMipmapMode::eNearest;
			break;
		case TextureSamplingFilterType::Linear:
			filter = vk::Filter::eLinear;
			mipmapMode = vk::SamplerMipmapMode::eLinear;
			break;
	}

	vk::SamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.flags = vk::SamplerCreateFlags();
	samplerCreateInfo.magFilter = filter;
	samplerCreateInfo.minFilter = filter;
	samplerCreateInfo.mipmapMode = mipmapMode;
	samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = vk::False;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	samplerCreateInfo.compareEnable = vk::False;
	samplerCreateInfo.compareOp = vk::CompareOp::eAlways;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerCreateInfo.unnormalizedCoordinates = vk::False;

	vk::Result result = device.createSampler(&samplerCreateInfo, nullptr, &sampler);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create framebuffer attachment sampler. Vulkan result: %d", result);
}
