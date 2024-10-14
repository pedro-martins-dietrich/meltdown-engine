#include <pch.hpp>
#include "ComputePipeline.hpp"

#include "../../Utils/Logger.hpp"

mtd::ComputePipeline::ComputePipeline(const vk::Device& device, const char* computeShaderFile)
	: device{device}, computeShader{computeShaderFile, device}
{
	createDescriptorSetLayouts();
	createPipelineLayout();
	createComputePipeline();
}

mtd::ComputePipeline::~ComputePipeline()
{
	device.destroyPipeline(computePipeline);
	device.destroyPipelineLayout(pipelineLayout);
}

// Starts the compute shader execution
void mtd::ComputePipeline::dispatchCompute
(
	const CommandHandler& commandHandler, glm::uvec3 workgroups
) const
{
	vk::CommandBuffer computeCommandBuffer = commandHandler.beginSingleTimeCommand();

	computeCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline);
	computeCommandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eCompute,
		pipelineLayout,
		0,
		1, &(descriptorSetHandlers[0].getSet(0)),
		0, nullptr
	);

	workgroups = glm::min(workgroups, glm::uvec3{65535U});
	computeCommandBuffer.dispatch(workgroups.x, workgroups.y, workgroups.z);

	commandHandler.endSingleTimeCommand(computeCommandBuffer);
}

// Configures the descriptor set handlers to be used
void mtd::ComputePipeline::createDescriptorSetLayouts()
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings(2);

	bindings[0].binding = 0;
	bindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
	bindings[0].pImmutableSamplers = nullptr;

	bindings[1].binding = 1;
	bindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
	bindings[1].pImmutableSamplers = nullptr;

	descriptorSetHandlers.emplace_back(device, bindings);
}

// Creates the layout for the pipeline
void mtd::ComputePipeline::createPipelineLayout()
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.reserve((descriptorSetHandlers.size()));
	for(const DescriptorSetHandler& descriptorSetHandler: descriptorSetHandlers)
		descriptorSetLayouts.push_back(descriptorSetHandler.getLayout());

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.flags = vk::PipelineLayoutCreateFlags();
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	vk::Result result =
		device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create compute pipeline layout. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created compute pipeline layout.");
}

// Creates the compute pipeline from the layout
void mtd::ComputePipeline::createComputePipeline()
{
	vk::PipelineShaderStageCreateInfo computeShaderStageCreateInfo{};
	computeShaderStageCreateInfo.flags = vk::PipelineShaderStageCreateFlags();
	computeShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eCompute;
	computeShaderStageCreateInfo.module = computeShader.getShaderModule();
	computeShaderStageCreateInfo.pName = "main";
	computeShaderStageCreateInfo.pSpecializationInfo = nullptr;

	vk::ComputePipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.flags = vk::PipelineCreateFlags();
	pipelineCreateInfo.stage = computeShaderStageCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = 0;

	vk::Result result =
		device.createComputePipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &computePipeline);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create compute pipeline. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created compute pipeline.");
}
