#include <pch.hpp>
#include "Pipeline.hpp"

#include "Builders/PipelineMapping.hpp"
#include "Builders/VertexInputBuilder.hpp"
#include "Builders/ColorBlendBuilder.hpp"
#include "Builders/DescriptorSetBuilder.hpp"
#include "../../Utils/Logger.hpp"

mtd::Pipeline::Pipeline
(
	const vk::Device& device,
	const PipelineInfo& info,
	const vk::DescriptorSetLayout& globalDescriptorSetLayout,
	vk::Extent2D extent,
	vk::RenderPass renderPass
) : device{device}, info{info}
{
	createDescriptorSetLayouts();
	loadShaderModules(info.vertexShaderPath.c_str(), info.fragmentShaderPath.c_str());
	createPipeline(extent, renderPass, globalDescriptorSetLayout);
}

mtd::Pipeline::~Pipeline()
{
	destroy();
}

mtd::Pipeline::Pipeline(Pipeline&& other) noexcept
	: device{other.device},
	info{std::move(other.info)},
	pipeline{std::move(other.pipeline)},
	pipelineLayout{std::move(other.pipelineLayout)},
	shaders{std::move(other.shaders)},
	descriptorSetHandlers{std::move(other.descriptorSetHandlers)},
	descriptorTypeCount{std::move(other.descriptorTypeCount)}
{
	other.pipeline = nullptr;
	other.pipelineLayout = nullptr;
}

// Recreates the pipeline
void mtd::Pipeline::recreate
(
	vk::Extent2D extent,
	vk::RenderPass renderPass,
	const vk::DescriptorSetLayout& globalDescriptorSetLayout
)
{
	destroy();
	createPipeline(extent, renderPass, globalDescriptorSetLayout);
}

// Allocates user descriptor set data in the descriptor pool
void mtd::Pipeline::configureUserDescriptorData(const Device& mtdDevice, const DescriptorPool& pool)
{
	if(descriptorSetHandlers.size() < 2) return;

	DescriptorSetHandler& descriptorSetHandler = descriptorSetHandlers[1];
	descriptorSetHandler.defineDescriptorSetsAmount(1);
	pool.allocateDescriptorSet(descriptorSetHandler);

	for(uint32_t binding = 0; binding < info.descriptorSetInfo.size(); binding++)
	{
		const DescriptorInfo& bindingInfo = info.descriptorSetInfo[binding];
		descriptorSetHandler.createDescriptorResources
		(
			mtdDevice,
			bindingInfo.totalDescriptorSize,
			PipelineMapping::mapBufferUsageFlags(bindingInfo.descriptorType),
			0, binding
		);
	}
	descriptorSetHandler.writeDescriptorSet(0);
}

// Updates the user descriptor data for the specified binding
void mtd::Pipeline::updateDescriptorData(uint32_t binding, const void* data) const
{
	if(descriptorSetHandlers.size() < 2 || descriptorSetHandlers[1].getSetCount() <= binding) return;

	descriptorSetHandlers[1].updateDescriptorData
	(
		0, binding, data, info.descriptorSetInfo[binding].totalDescriptorSize
	);
}

// Binds the pipeline to the command buffer
void mtd::Pipeline::bind(const vk::CommandBuffer& commandBuffer) const
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
}

// Binds per pipeline descriptors
void mtd::Pipeline::bindPipelineDescriptors(const vk::CommandBuffer& commandBuffer) const
{
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

// Binds per mesh descriptors
void mtd::Pipeline::bindMeshDescriptors(const vk::CommandBuffer& commandBuffer, uint32_t index) const
{
	commandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eGraphics,
		pipelineLayout,
		1,
		1, &(descriptorSetHandlers[0].getSet(index)),
		0, nullptr
	);
}

// Loads the pipeline shader modules
void mtd::Pipeline::loadShaderModules(const char* vertexShaderPath, const char* fragmentShaderPath)
{
	shaders.reserve(2);
	shaders.emplace_back(vertexShaderPath, device);
	shaders.emplace_back(fragmentShaderPath, device);
}

// Creates the graphics pipeline
void mtd::Pipeline::createPipeline
(
	vk::Extent2D extent, vk::RenderPass renderPass, const vk::DescriptorSetLayout& globalDescriptorSetLayout
)
{
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStagesCreateInfos;
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
	setVertexShader(shaderStagesCreateInfos, shaders[0]);
	setViewport(viewportCreateInfo, viewport, scissor, extent);
	setRasterizer(rasterizationCreateInfo);
	setFragmentShader(shaderStagesCreateInfos, shaders[1]);
	setMultisampling(multisampleCreateInfo);
	setDepthStencil(depthStencilCreateInfo);
	ColorBlendBuilder::setColorBlending(info.useTransparency, colorBlendCreateInfo, colorBlendAttachment);

	createPipelineLayout(globalDescriptorSetLayout);

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

	vk::Result result = device.createGraphicsPipelines(nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create graphics pipeline. Vulkan result: %d", result);
		return;
	}
	LOG_INFO("Created graphics pipeline.\n");
}

// Configures the descriptor set handlers to be used
void mtd::Pipeline::createDescriptorSetLayouts()
{
	descriptorSetHandlers.reserve(info.descriptorSetInfo.size() == 0 ? 1 : 2);

	bool hasFloatData = !info.materialFloatDataTypes.empty();
	bool hasTextures = !info.materialTextureTypes.empty();

	uint32_t binding = 0;
	std::vector<vk::DescriptorSetLayoutBinding> bindings
	(
		static_cast<uint32_t>(hasFloatData) + static_cast<uint32_t>(hasTextures)
	);

	if(hasFloatData)
	{
		bindings[binding].binding = binding;
		bindings[binding].descriptorType = vk::DescriptorType::eUniformBuffer;
		bindings[binding].descriptorCount = 1;
		bindings[binding].stageFlags = vk::ShaderStageFlagBits::eFragment;
		bindings[binding].pImmutableSamplers = nullptr;

		binding++;
	}
	if(hasTextures)
	{
		bindings[binding].binding = binding;
		bindings[binding].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		bindings[binding].descriptorCount = static_cast<uint32_t>(info.materialTextureTypes.size());
		bindings[binding].stageFlags = vk::ShaderStageFlagBits::eFragment;
		bindings[binding].pImmutableSamplers = nullptr;
	}

	descriptorSetHandlers.emplace_back(device, bindings);

	if(info.descriptorSetInfo.size() == 0) return;
	bindings.clear();

	DescriptorSetBuilder::buildDescriptorSetLayout(bindings, info.descriptorSetInfo, descriptorTypeCount);
	descriptorSetHandlers.emplace_back(device, bindings);
}

// Sets the input assembly create info
void mtd::Pipeline::setInputAssembly(vk::PipelineInputAssemblyStateCreateInfo& inputAssemblyInfo) const
{
	inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
	inputAssemblyInfo.topology = PipelineMapping::mapPrimitiveTopology(info.primitiveTopology);
	inputAssemblyInfo.primitiveRestartEnable = vk::False;
}

// Sets the vertex shader stage create info
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

// Sets the viewport create info
void mtd::Pipeline::setViewport
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
void mtd::Pipeline::setRasterizer(vk::PipelineRasterizationStateCreateInfo& rasterizationInfo) const
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

// Sets the fragment shader stage create info
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

// Sets the multisample create info
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

// Sets the depth stencil create info
void mtd::Pipeline::setDepthStencil(vk::PipelineDepthStencilStateCreateInfo& depthStencilInfo) const
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

// Creates the layout for the pipeline
void mtd::Pipeline::createPipelineLayout(const vk::DescriptorSetLayout& globalDescriptorSetLayout)
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
		LOG_ERROR("Failed to create pipeline layout. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created pipeline layout.");
}

// Clears pipeline objects
void mtd::Pipeline::destroy()
{
	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(pipelineLayout);
}
