#include <pch.hpp>
#include "RasterizationPipeline.hpp"

#include "Builders/PipelineMapping.hpp"
#include "Builders/VertexInputBuilder.hpp"
#include "Builders/ColorBlendBuilder.hpp"
#include "Builders/DescriptorSetBuilder.hpp"
#include "../../Utils/Logger.hpp"

mtd::RasterizationPipeline::RasterizationPipeline
(
	const vk::Device& device,
	const DescriptorManager& descriptorManager,
	const RasterizationPipelineInfo& info,
	const vk::DescriptorSetLayout& globalDescriptorSetLayout,
	vk::Extent2D extent,
	vk::RenderPass renderPass
) : Pipeline{device, descriptorManager, info}
{
	createDescriptorSetLayouts();
	loadShaderModules();
	createPipelineLayout(globalDescriptorSetLayout);
	createPipeline(extent, renderPass);
}

mtd::RasterizationPipeline::RasterizationPipeline(RasterizationPipeline&& other) noexcept
	: Pipeline{std::move(other)}
{}

void mtd::RasterizationPipeline::recreate(vk::Extent2D extent, vk::RenderPass renderPass)
{
	device.destroyPipeline(pipeline);
	createPipeline(extent, renderPass);
}

void mtd::RasterizationPipeline::bind(vk::CommandBuffer commandBuffer) const
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	std::vector<vk::DescriptorSet> descriptorSets;
	descriptorSets.reserve(info.descriptorSetIDs.size());
	for(DescriptorSetID setID: info.descriptorSetIDs)
		descriptorSets.emplace_back(descriptorManager.getSet(setID));

	if(descriptorSets.size() == 0U) return;

	commandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eGraphics,
		pipelineLayout,
		1U,
		static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(),
		0U, nullptr
	);
}

void mtd::RasterizationPipeline::pushConstant(vk::CommandBuffer commandBuffer, const uint32_t& constantData) const
{
	commandBuffer.pushConstants
	(
		pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0U, sizeof(uint32_t), &constantData
	);
}

void mtd::RasterizationPipeline::loadShaderModules()
{
	shaders.reserve(2);
	shaders.emplace_back(device, vk::ShaderStageFlagBits::eVertex, info.vertexShaderPath.c_str());
	shaders.emplace_back(device, vk::ShaderStageFlagBits::eFragment, info.fragmentShaderPath.c_str());
}

void mtd::RasterizationPipeline::createPipelineLayout(vk::DescriptorSetLayout globalDescriptorSetLayout)
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{globalDescriptorSetLayout};
	for(DescriptorSetID setID: info.descriptorSetIDs)
		descriptorSetLayouts.emplace_back(descriptorManager.getLayout(setID));

	vk::PushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
	pushConstantRange.offset = 0U;
	pushConstantRange.size = sizeof(uint32_t);

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.flags = vk::PipelineLayoutCreateFlags();
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1U;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	vk::Result result = device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create pipeline layout. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created pipeline layout.");
}

void mtd::RasterizationPipeline::createPipeline(vk::Extent2D extent, vk::RenderPass renderPass)
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

	VertexInputBuilder::setVertexInput(info.associatedMeshType, vertexInputCreateInfo);
	setInputAssembly(inputAssemblyCreateInfo);
	setViewport(viewportCreateInfo, viewport, scissor, extent);
	setRasterizer(rasterizationCreateInfo);
	setMultisampling(multisampleCreateInfo);
	setDepthStencil(depthStencilCreateInfo);
	ColorBlendBuilder::setColorBlending(info.useTransparency, colorBlendCreateInfo, colorBlendAttachment);

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
		LOG_ERROR("Failed to create graphics pipeline. Vulkan result: %d", result);
		return;
	}
	LOG_INFO("Created graphics pipeline.\n");
}

void mtd::RasterizationPipeline::createDescriptorSetLayouts()
{
	userDescriptorSetIndex = 1U;
	if(info.descriptorSetInfo.size() == 0) return;
	
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	DescriptorSetBuilder::buildDescriptorSetLayout(bindings, info.descriptorSetInfo, descriptorTypeCount);
	descriptorSetHandlers.emplace_back(device, bindings);
}

void mtd::RasterizationPipeline::setInputAssembly(vk::PipelineInputAssemblyStateCreateInfo& inputAssemblyInfo) const
{
	inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
	inputAssemblyInfo.topology = PipelineMapping::mapPrimitiveTopology(info.primitiveTopology);
	inputAssemblyInfo.primitiveRestartEnable = vk::False;
}

void mtd::RasterizationPipeline::setViewport
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

void mtd::RasterizationPipeline::setRasterizer(vk::PipelineRasterizationStateCreateInfo& rasterizationInfo) const
{
	rasterizationInfo.flags = vk::PipelineRasterizationStateCreateFlags();
	rasterizationInfo.depthClampEnable = vk::False;
	rasterizationInfo.rasterizerDiscardEnable = vk::False;
	rasterizationInfo.polygonMode = PipelineMapping::mapPolygonMode(info.primitiveTopology);
	rasterizationInfo.cullMode = PipelineMapping::mapCullModeFlags(info.faceCulling);
	rasterizationInfo.frontFace = PipelineMapping::mapFrontFace(info.faceCulling);
	rasterizationInfo.depthBiasEnable = vk::False;
	rasterizationInfo.depthBiasConstantFactor = 0.0f;
	rasterizationInfo.depthBiasClamp = 0.0f;
	rasterizationInfo.depthBiasSlopeFactor = 0.0f;
	rasterizationInfo.lineWidth = 1.0f;
}

void mtd::RasterizationPipeline::setMultisampling(vk::PipelineMultisampleStateCreateInfo& multisampleInfo) const
{
	multisampleInfo.flags = vk::PipelineMultisampleStateCreateFlags();
	multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
	multisampleInfo.sampleShadingEnable = vk::False;
	multisampleInfo.minSampleShading = 0.0f;
	multisampleInfo.pSampleMask = nullptr;
	multisampleInfo.alphaToCoverageEnable = vk::False;
	multisampleInfo.alphaToOneEnable = vk::False;
}

void mtd::RasterizationPipeline::setDepthStencil(vk::PipelineDepthStencilStateCreateInfo& depthStencilInfo) const
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
