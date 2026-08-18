#include <pch.hpp>
#include "FramebufferPipeline.hpp"

#include "Builders/ColorBlendBuilder.hpp"
#include "../../Utils/Logger.hpp"

mtd::FramebufferPipeline::FramebufferPipeline
(
	const vk::Device& device,
	const DescriptorManager& descriptorManager,
	const FramebufferPipelineInfo& info,
	vk::Extent2D extent,
	vk::RenderPass renderPass
) : Pipeline{device, descriptorManager, info}
{
	createDescriptorSetLayouts();
	loadShaderModules();
	createPipelineLayout();
	createPipeline(extent, renderPass);
}

mtd::FramebufferPipeline::FramebufferPipeline(FramebufferPipeline&& other) noexcept
	: Pipeline{std::move(other)}
{}

uint32_t mtd::FramebufferPipeline::getImageDescriptorsCount() const
{
	return static_cast<uint32_t>(info.inputAttachments.size()
		+ info.rayTracingStorageImages.size() + info.computeStorageImages.size());
}

void mtd::FramebufferPipeline::recreate(vk::Extent2D extent, vk::RenderPass renderPass)
{
	device.destroyPipeline(pipeline);
	createPipeline(extent, renderPass);
}

void mtd::FramebufferPipeline::bind(const vk::CommandBuffer& commandBuffer) const
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	std::vector<vk::DescriptorSet> descriptorSets;
	descriptorSets.reserve(info.descriptorSetIDs.size() + 1);
	for(DescriptorSetID setID: info.descriptorSetIDs)
		descriptorSets.emplace_back(descriptorManager.getSet(setID));
	descriptorSets.emplace_back(descriptorSetHandlers[0].getSet());

	commandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eGraphics,
		pipelineLayout,
		0U,
		static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(),
		0U, nullptr
	);
}

void mtd::FramebufferPipeline::updateInputImagesDescriptors
(
	const std::vector<Framebuffer>& framebuffers,
	const std::vector<ComputePipeline>& computePipelines,
	const std::vector<RayTracingPipeline>& rayTracingPipelines
)
{
	DescriptorSetHandler& descriptorSetHandler = descriptorSetHandlers[0];
	uint32_t binding = 0U;

	for(const AttachmentIdentifier& attachmentIdentifier: info.inputAttachments)
	{
		uint32_t fbIndex = attachmentIdentifier.framebufferIndex;
		uint32_t attachmentIndex = attachmentIdentifier.attachmentIndex;

		assert(fbIndex < framebuffers.size() && "Framebuffer index out of bounds.");
		framebuffers[fbIndex].configureAttachmentAsDescriptor(descriptorSetHandler, binding, attachmentIndex);
		binding++;
	}

	for(uint32_t computePipelineIndex: info.computeStorageImages)
	{
		assert(computePipelineIndex < computePipelines.size() && "Compute pipeline index out of bounds.");
		computePipelines[computePipelineIndex].shareRenderTargetImageDescriptor(descriptorSetHandler, binding);
		binding++;
	}

	for(uint32_t rtPipelineIndex: info.rayTracingStorageImages)
	{
		assert(rtPipelineIndex < rayTracingPipelines.size() && "Ray tracing pipeline index out of bounds.");
		rayTracingPipelines[rtPipelineIndex].shareRenderTargetImageDescriptor(descriptorSetHandler, binding);
		binding++;
	}

	descriptorSetHandler.writeDescriptorSet();
}

void mtd::FramebufferPipeline::loadShaderModules()
{
	shaders.reserve(2);
	shaders.emplace_back(device, vk::ShaderStageFlagBits::eVertex, info.vertexShaderPath.c_str());
	shaders.emplace_back(device, vk::ShaderStageFlagBits::eFragment, info.fragmentShaderPath.c_str());
}

void mtd::FramebufferPipeline::createPipelineLayout()
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.reserve(info.descriptorLayoutIDs.size() + 1);
	for(DescriptorLayoutID layoutID: info.descriptorLayoutIDs)
		descriptorSetLayouts.emplace_back(descriptorManager.getLayout(layoutID));
	for(const DescriptorSetHandler& descriptorSetHandler: descriptorSetHandlers)
		descriptorSetLayouts.emplace_back(descriptorSetHandler.getLayout());

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.flags = vk::PipelineLayoutCreateFlags();
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0U;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	vk::Result result = device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create framebuffer pipeline layout. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created framebuffer pipeline layout.");
}

void mtd::FramebufferPipeline::createPipeline(vk::Extent2D extent, vk::RenderPass renderPass)
{
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos;
	shaderStageCreateInfos.reserve(shaders.size());
	for(const ShaderModule& shader: shaders)
		shaderStageCreateInfos.emplace_back(shader.generatePipelineShaderCreateInfo());

	vk::Viewport viewport{};
	vk::Rect2D scissor{};
	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};

	vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
	vk::PipelineViewportStateCreateInfo viewportCreateInfo{};
	vk::PipelineRasterizationStateCreateInfo rasterizationCreateInfo{};
	vk::PipelineMultisampleStateCreateInfo multisampleCreateInfo{};
	vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
	vk::PipelineColorBlendStateCreateInfo colorBlendCreateInfo{};

	setVertexInput(vertexInputCreateInfo);
	setInputAssembly(inputAssemblyCreateInfo);
	setViewport(viewportCreateInfo, viewport, scissor, extent);
	setRasterizer(rasterizationCreateInfo);
	setMultisampling(multisampleCreateInfo);
	setDepthStencil(depthStencilCreateInfo);
	ColorBlendBuilder::setColorBlending(false, colorBlendCreateInfo, colorBlendAttachment);

	vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.flags = vk::PipelineCreateFlags();
	graphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
	graphicsPipelineCreateInfo.pStages = shaderStageCreateInfos.data();
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	graphicsPipelineCreateInfo.pTessellationState = nullptr;
	graphicsPipelineCreateInfo.pViewportState = &viewportCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = nullptr;
	graphicsPipelineCreateInfo.layout = pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.subpass = 0U;
	graphicsPipelineCreateInfo.basePipelineHandle = nullptr;
	graphicsPipelineCreateInfo.basePipelineIndex = 0;

	vk::Result result = device.createGraphicsPipelines(nullptr, 1U, &graphicsPipelineCreateInfo, nullptr, &pipeline);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create framebuffer pipeline. Vulkan result: %d", result);
		return;
	}
	LOG_INFO("Created framebuffer pipeline.\n");
}

void mtd::FramebufferPipeline::createDescriptorSetLayouts()
{
	uint32_t imageDescriptorsCount = info.inputAttachments.size()
		+ info.rayTracingStorageImages.size() + info.computeStorageImages.size();
	std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(imageDescriptorsCount);
	for(uint32_t i = 0U; i < layoutBindings.size(); i++)
	{
		layoutBindings[i].binding = i;
		layoutBindings[i].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		layoutBindings[i].descriptorCount = 1U;
		layoutBindings[i].stageFlags = vk::ShaderStageFlagBits::eFragment;
		layoutBindings[i].pImmutableSamplers = nullptr;
	}

	descriptorSetHandlers.emplace_back(device, layoutBindings);
}

void mtd::FramebufferPipeline::setVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo) const
{
	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = 0U;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0U;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;
}

void mtd::FramebufferPipeline::setInputAssembly(vk::PipelineInputAssemblyStateCreateInfo& inputAssemblyInfo) const
{
	inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
	inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleFan;
	inputAssemblyInfo.primitiveRestartEnable = vk::False;
}

void mtd::FramebufferPipeline::setViewport
(
	vk::PipelineViewportStateCreateInfo& viewportInfo,
	vk::Viewport& viewport,
	vk::Rect2D& scissor,
	vk::Extent2D extent
) const
{
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = extent;

	viewportInfo.flags = vk::PipelineViewportStateCreateFlags();
	viewportInfo.viewportCount = 1U;
	viewportInfo.pViewports = &viewport;
	viewportInfo.scissorCount = 1U;
	viewportInfo.pScissors = &scissor;
}

void mtd::FramebufferPipeline::setRasterizer(vk::PipelineRasterizationStateCreateInfo& rasterizationInfo) const
{
	rasterizationInfo.flags = vk::PipelineRasterizationStateCreateFlags();
	rasterizationInfo.depthClampEnable = vk::False;
	rasterizationInfo.rasterizerDiscardEnable = vk::False;
	rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
	rasterizationInfo.cullMode = vk::CullModeFlagBits::eBack;
	rasterizationInfo.frontFace = vk::FrontFace::eCounterClockwise;
	rasterizationInfo.depthBiasEnable = vk::False;
	rasterizationInfo.depthBiasConstantFactor = 0.0f;
	rasterizationInfo.depthBiasClamp = 0.0f;
	rasterizationInfo.depthBiasSlopeFactor = 0.0f;
	rasterizationInfo.lineWidth = 1.0f;
}

void mtd::FramebufferPipeline::setMultisampling(vk::PipelineMultisampleStateCreateInfo& multisampleInfo) const
{
	multisampleInfo.flags = vk::PipelineMultisampleStateCreateFlags();
	multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
	multisampleInfo.sampleShadingEnable = vk::False;
	multisampleInfo.minSampleShading = 0.0f;
	multisampleInfo.pSampleMask = nullptr;
	multisampleInfo.alphaToCoverageEnable = vk::False;
	multisampleInfo.alphaToOneEnable = vk::False;
}

void mtd::FramebufferPipeline::setDepthStencil(vk::PipelineDepthStencilStateCreateInfo& depthStencilInfo) const
{
	depthStencilInfo.flags = vk::PipelineDepthStencilStateCreateFlags();
	depthStencilInfo.depthTestEnable = vk::True;
	depthStencilInfo.depthWriteEnable = vk::True;
	depthStencilInfo.depthCompareOp = vk::CompareOp::eLess;
	depthStencilInfo.depthBoundsTestEnable = vk::False;
	depthStencilInfo.stencilTestEnable = vk::False;
	depthStencilInfo.front = vk::StencilOpState{};
	depthStencilInfo.back = vk::StencilOpState{};
	depthStencilInfo.minDepthBounds = 0.0f;
	depthStencilInfo.maxDepthBounds = 0.0f;
}
