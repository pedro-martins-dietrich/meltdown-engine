#include <pch.hpp>
#include "FramebufferPipeline.hpp"

#include "Builders/PipelineMapping.hpp"
#include "Builders/ColorBlendBuilder.hpp"
#include "Builders/DescriptorSetBuilder.hpp"
#include "../../Utils/Logger.hpp"

mtd::FramebufferPipeline::FramebufferPipeline
(
	const vk::Device& device,
	const FramebufferPipelineInfo& info,
	const vk::DescriptorSetLayout& globalDescriptorSetLayout,
	vk::Extent2D extent,
	vk::RenderPass renderPass
) : Pipeline{device, info}
{
	createDescriptorSetLayouts();
	loadShaderModules();
	createPipelineLayout(globalDescriptorSetLayout);
	createPipeline(extent, renderPass);
}

mtd::FramebufferPipeline::FramebufferPipeline(FramebufferPipeline&& other) noexcept
	: Pipeline{std::move(other)}
{}

// Recreates the framebuffer pipeline
void mtd::FramebufferPipeline::recreate(vk::Extent2D extent, vk::RenderPass renderPass)
{
	device.destroyPipeline(pipeline);
	createPipeline(extent, renderPass);
}

// Binds the pipeline to the command buffer
void mtd::FramebufferPipeline::bind(const vk::CommandBuffer& commandBuffer) const
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	commandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eGraphics,
		pipelineLayout,
		1,
		1, &(descriptorSetHandlers[0].getSet(0)),
		0, nullptr
	);

	if(descriptorSetHandlers.size() == 1 || descriptorSetHandlers[1].getSetCount() == 0) return;

	commandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eGraphics,
		pipelineLayout,
		2,
		1, &(descriptorSetHandlers[1].getSet(0)),
		0, nullptr
	);
}

// Updates all the input images descriptors
void mtd::FramebufferPipeline::updateInputImagesDescriptors
(
	const std::vector<Framebuffer>& framebuffers, const std::vector<RayTracingPipeline>& rayTracingPipelines
)
{
	DescriptorSetHandler& descriptorSetHandler = descriptorSetHandlers[0];
	uint32_t binding = 0;

	for(const AttachmentIdentifier& attachmentIdentifier: info.inputAttachments)
	{
		uint32_t fbIndex = attachmentIdentifier.framebufferIndex;
		uint32_t attachmentIndex = attachmentIdentifier.attachmentIndex;

		assert(fbIndex < framebuffers.size() && "Framebuffer index out of bounds.");
		framebuffers[fbIndex].configureAttachmentAsDescriptor(descriptorSetHandler, binding, attachmentIndex);
		binding++;
	}

	for(uint32_t rtPipelineIndex: info.rayTracingStorageImages)
	{
		assert(rtPipelineIndex < rayTracingPipelines.size() && "Ray tracing pipeline index out of bounds.");
		rayTracingPipelines[rtPipelineIndex].shareRenderTargetImageDescriptor(descriptorSetHandler, binding);
		binding++;
	}

	descriptorSetHandler.writeDescriptorSet(0);
}

// Loads the pipeline shader modules
void mtd::FramebufferPipeline::loadShaderModules()
{
	shaders.reserve(2);
	shaders.emplace_back(device, vk::ShaderStageFlagBits::eVertex, info.vertexShaderPath.c_str());
	shaders.emplace_back(device, vk::ShaderStageFlagBits::eFragment, info.fragmentShaderPath.c_str());
}

// Creates the layout for the framebuffer pipeline
void mtd::FramebufferPipeline::createPipelineLayout(const vk::DescriptorSetLayout& globalDescriptorSetLayout)
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{globalDescriptorSetLayout};
	for(const DescriptorSetHandler& descriptorSetHandler: descriptorSetHandlers)
		descriptorSetLayouts.push_back(descriptorSetHandler.getLayout());

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.flags = vk::PipelineLayoutCreateFlags();
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	vk::Result result = device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create framebuffer pipeline layout. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created framebuffer pipeline layout.");
}

// Creates the graphics pipeline
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
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.basePipelineHandle = nullptr;
	graphicsPipelineCreateInfo.basePipelineIndex = 0;

	vk::Result result = device.createGraphicsPipelines(nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create framebuffer pipeline. Vulkan result: %d", result);
		return;
	}
	LOG_INFO("Created framebuffer pipeline.\n");
}

// Configures the descriptor set handlers to be used
void mtd::FramebufferPipeline::createDescriptorSetLayouts()
{
	descriptorSetHandlers.reserve(info.descriptorSetInfo.size() == 0 ? 1 : 2);

	uint32_t imageDescriptorsCount = info.inputAttachments.size() + info.rayTracingStorageImages.size();
	std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(imageDescriptorsCount);
	for(uint32_t i = 0; i < layoutBindings.size(); i++)
	{
		layoutBindings[i].binding = i;
		layoutBindings[i].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		layoutBindings[i].descriptorCount = 1;
		layoutBindings[i].stageFlags = vk::ShaderStageFlagBits::eFragment;
		layoutBindings[i].pImmutableSamplers = nullptr;
	}

	descriptorSetHandlers.emplace_back(device, layoutBindings);

	if(info.descriptorSetInfo.size() == 0) return;
	layoutBindings.clear();

	DescriptorSetBuilder::buildDescriptorSetLayout(layoutBindings, info.descriptorSetInfo, descriptorTypeCount);
	descriptorSetHandlers.emplace_back(device, layoutBindings);
}

// Sets the vertex input create info
void mtd::FramebufferPipeline::setVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo) const
{
	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;
}

// Sets the input assembly create info
void mtd::FramebufferPipeline::setInputAssembly(vk::PipelineInputAssemblyStateCreateInfo& inputAssemblyInfo) const
{
	inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
	inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleFan;
	inputAssemblyInfo.primitiveRestartEnable = vk::False;
}

// Sets the viewport create info
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
	viewportInfo.viewportCount = 1;
	viewportInfo.pViewports = &viewport;
	viewportInfo.scissorCount = 1;
	viewportInfo.pScissors = &scissor;
}

// Sets the rasterization create info
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

// Sets the multisample create info
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

// Sets the depth stencil create info
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
