#include "Pipeline.hpp"

#include "../../Utils/Logger.hpp"
#include "../Mesh/Mesh.hpp"

mtd::Pipeline::Pipeline(const vk::Device& device, Swapchain& swapchain)
	: device{device}
{
	createPipeline(swapchain);
}

mtd::Pipeline::~Pipeline()
{
	destroy();
}

// Recreates the pipeline
void mtd::Pipeline::recreate(Swapchain& swapchain)
{
	destroy();
	createPipeline(swapchain);
}

// Creates the graphics pipeline
void mtd::Pipeline::createPipeline(Swapchain& swapchain)
{
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStagesCreateInfos;
	vk::Viewport viewport{};
	vk::Rect2D scissor{};
	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};

	ShaderModule vertexShaderModule{"default.vert.spv", device};
	ShaderModule fragmentShaderModule{"default.frag.spv", device};

	vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
	vk::PipelineViewportStateCreateInfo viewportCreateInfo{};
	vk::PipelineRasterizationStateCreateInfo rasterizationCreateInfo{};
	vk::PipelineMultisampleStateCreateInfo multisampleCreateInfo{};
	vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
	vk::PipelineColorBlendStateCreateInfo colorBlendCreateInfo{};

	setVertexInput(vertexInputCreateInfo);
	setInputAssembly(inputAssemblyCreateInfo);
	setVertexShader(shaderStagesCreateInfos, vertexShaderModule);
	setViewport(viewportCreateInfo, viewport, scissor, swapchain);
	setRasterizer(rasterizationCreateInfo);
	setFragmentShader(shaderStagesCreateInfos, fragmentShaderModule);
	setMultisampling(multisampleCreateInfo);
	setDepthStencil(depthStencilCreateInfo);
	setColorBlending(colorBlendCreateInfo, colorBlendAttachment);

	createPipelineLayout();
	createRenderPass(swapchain);

	vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.flags = vk::PipelineCreateFlags();
	graphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStagesCreateInfos.size());
	graphicsPipelineCreateInfo.pStages = shaderStagesCreateInfos.data();
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

	vk::Result result = device.createGraphicsPipelines
	(
		nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline
	);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create graphics pipeline. Vulkan result: %d", result);
		return;
	}
	LOG_INFO("Created graphics pipeline.\n");
}

// Sets create info for the vertex input
void mtd::Pipeline::setVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo) const
{
	const vk::VertexInputBindingDescription& bindingDescription =
		Mesh::getInputBindingDescription();
	const std::vector<vk::VertexInputAttributeDescription>& attributeDescriptions =
		Mesh::getInputAttributeDescriptions();

	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
}

// Sets create info for the input assembly
void mtd::Pipeline::setInputAssembly
(
	vk::PipelineInputAssemblyStateCreateInfo& inputAssemblyInfo
) const
{
	inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
	inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssemblyInfo.primitiveRestartEnable = vk::False;
}

// Sets create info for the vertex shader stage
void mtd::Pipeline::setVertexShader
(
	std::vector<vk::PipelineShaderStageCreateInfo>& shaderStageInfos,
	const ShaderModule& vertexShaderModule
) const
{
	shaderStageInfos.emplace_back
	(
		vk::PipelineShaderStageCreateFlags(),	// flags
		vk::ShaderStageFlagBits::eVertex,		// stage
		vertexShaderModule.getShaderModule(),	// module
		"main",									// pName
		nullptr									// pSpecializationInfo
	);
}

// Sets create info for the viewport
void mtd::Pipeline::setViewport
(
	vk::PipelineViewportStateCreateInfo& viewportInfo,
	vk::Viewport& viewport,
	vk::Rect2D& scissor,
	const Swapchain& swapchain
) const
{
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchain.getExtent().width);
	viewport.height = static_cast<float>(swapchain.getExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = swapchain.getExtent();

	viewportInfo.flags = vk::PipelineViewportStateCreateFlags();
	viewportInfo.viewportCount = 1;
	viewportInfo.pViewports = &viewport;
	viewportInfo.scissorCount = 1;
	viewportInfo.pScissors = &scissor;
}

// Sets create info for the rasterization
void mtd::Pipeline::setRasterizer
(
	vk::PipelineRasterizationStateCreateInfo& rasterizationInfo
) const
{
	rasterizationInfo.flags = vk::PipelineRasterizationStateCreateFlags();
	rasterizationInfo.depthClampEnable = vk::False;
	rasterizationInfo.rasterizerDiscardEnable = vk::False;
	rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
	rasterizationInfo.cullMode = vk::CullModeFlagBits::eNone;
	rasterizationInfo.frontFace = vk::FrontFace::eCounterClockwise;
	rasterizationInfo.depthBiasEnable = vk::False;
	rasterizationInfo.depthBiasConstantFactor = 0.0f;
	rasterizationInfo.depthBiasClamp = 0.0f;
	rasterizationInfo.depthBiasSlopeFactor = 0.0f;
	rasterizationInfo.lineWidth = 1.0f;
}

// Sets create info for the fragment shader stage
void mtd::Pipeline::setFragmentShader
(
	std::vector<vk::PipelineShaderStageCreateInfo>& shaderStageInfos,
	const ShaderModule& fragmentShaderModule
) const
{
	shaderStageInfos.emplace_back
	(
		vk::PipelineShaderStageCreateFlags(),	// flags
		vk::ShaderStageFlagBits::eFragment,		// stage
		fragmentShaderModule.getShaderModule(),	// module
		"main",									// pName
		nullptr									// pSpecializationInfo
	);
}

// Sets create info for the multisample
void mtd::Pipeline::setMultisampling(vk::PipelineMultisampleStateCreateInfo& multisampleInfo) const
{
	multisampleInfo.flags = vk::PipelineMultisampleStateCreateFlags();
	multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
	multisampleInfo.sampleShadingEnable = vk::False;
	multisampleInfo.minSampleShading = 0.0f;
	multisampleInfo.pSampleMask = nullptr;
	multisampleInfo.alphaToCoverageEnable = vk::False;
	multisampleInfo.alphaToOneEnable = vk::False;
}

// Sets create info for the depth stencil
void mtd::Pipeline::setDepthStencil
(
	vk::PipelineDepthStencilStateCreateInfo& depthStencilInfo
) const
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

// Sets create info for the color blend
void mtd::Pipeline::setColorBlending
(
	vk::PipelineColorBlendStateCreateInfo& colorBlendInfo,
	vk::PipelineColorBlendAttachmentState& colorBlendAttachment
) const
{
	colorBlendAttachment.blendEnable = vk::False;
	colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eZero;
	colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
	colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
	colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eZero;
	colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
		vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eA;

	colorBlendInfo.flags = vk::PipelineColorBlendStateCreateFlags();
	colorBlendInfo.logicOpEnable = vk::False;
	colorBlendInfo.logicOp = vk::LogicOp::eCopy;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachment;
	colorBlendInfo.blendConstants = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};
}

// Creates the layout for the pipeline
void mtd::Pipeline::createPipelineLayout()
{
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(CameraMatrices);

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.flags = vk::PipelineLayoutCreateFlags();
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	vk::Result result = device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create pipeline layout. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created pipeline layout.");
}

// Creates pipeline render pass
void mtd::Pipeline::createRenderPass(Swapchain& swapchain)
{
	vk::AttachmentDescription colorAttachmentDescription{};
	colorAttachmentDescription.flags = vk::AttachmentDescriptionFlags();
	colorAttachmentDescription.format = swapchain.getColorFormat();
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
	depthAttachmentDescription.format = swapchain.getDepthFormat();
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
	swapchain.createFramebuffers(renderPass);
}

// Clears pipeline objects
void mtd::Pipeline::destroy()
{
	device.destroyPipeline(pipeline);
	device.destroyRenderPass(renderPass);
	device.destroyPipelineLayout(pipelineLayout);
}
