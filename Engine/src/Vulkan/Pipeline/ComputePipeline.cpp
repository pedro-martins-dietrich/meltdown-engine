#include <pch.hpp>
#include "ComputePipeline.hpp"

#include "Builders/DescriptorSetBuilder.hpp"
#include "../../Utils/Logger.hpp"

mtd::ComputePipeline::ComputePipeline
(
	const Device& mtdDevice,
	const ComputePipelineInfo& info,
	const vk::DescriptorSetLayout& globalDescriptorSetLayout,
	vk::Extent2D swapchainExtent
) : Pipeline{mtdDevice.getDevice(), info}, outputImage{mtdDevice.getDevice()}
{
	loadShaderModule();
	createDescriptorSetLayouts();
	createPipelineLayout(globalDescriptorSetLayout);
	createComputePipeline();
	createStorageImages(mtdDevice, swapchainExtent);
}

mtd::ComputePipeline::ComputePipeline(ComputePipeline&& other) noexcept
	: Pipeline{std::move(other)},
	outputImage{std::move(other.outputImage)},
	images{std::move(other.images)},
	currentRenderTargerImageIndex{other.currentRenderTargerImageIndex},
	windowResolutionDependant{other.windowResolutionDependant},
	pushConstantData{other.pushConstantData}
{}

void mtd::ComputePipeline::dispatchCompute(const vk::CommandBuffer& commandBuffer) const
{
	outputImage.transitionImageLayout(commandBuffer, vk::ImageLayout::eGeneral);
	for(const Image& image: images)
		image.transitionImageLayout(commandBuffer, vk::ImageLayout::eGeneral);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
	commandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eCompute,
		pipelineLayout,
		1,
		1, &(descriptorSetHandlers[0].getSet(0)),
		0, nullptr
	);

	if(descriptorSetHandlers.size() > 1 && descriptorSetHandlers[1].getSetCount() > 0)
	{
		commandBuffer.bindDescriptorSets
		(
			vk::PipelineBindPoint::eCompute,
			pipelineLayout,
			2,
			1, &(descriptorSetHandlers[1].getSet(0)),
			0, nullptr
		);
	}

	commandBuffer.pushConstants
	(
		pipelineLayout,
		vk::ShaderStageFlagBits::eCompute,
		0, sizeof(ComputeShaderPushConstantData), &pushConstantData
	);

	commandBuffer.dispatch(info.workgroups.x, info.workgroups.y, info.workgroups.z);

	pushConstantData.iterationCounter++;

	outputImage.transitionImageLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);
}

void mtd::ComputePipeline::configurePipelineDescriptorSet()
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	outputImage.defineDescriptorImageInfo(&descriptorImageInfo);
	descriptorImageInfo.imageLayout = vk::ImageLayout::eGeneral;
	descriptorSetHandlers[0].createImageDescriptorResources(0, 0, descriptorImageInfo);

	for(uint32_t i = 0; i < images.size(); i++)
	{
		images[i].defineDescriptorImageInfo(&descriptorImageInfo);
		descriptorImageInfo.imageLayout = vk::ImageLayout::eGeneral;
		descriptorSetHandlers[0].createImageDescriptorResources(0, i + 1U, descriptorImageInfo);
	}

	descriptorSetHandlers[0].writeDescriptorSet(0);
}

void mtd::ComputePipeline::shareRenderTargetImageDescriptor
(
	DescriptorSetHandler& descriptorSetHandler, uint32_t binding
) const
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	outputImage.defineDescriptorImageInfo(&descriptorImageInfo);
	descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	descriptorSetHandler.createImageDescriptorResources(0, binding, descriptorImageInfo);
}

void mtd::ComputePipeline::resize(const Device& mtdDevice, vk::Extent2D swapchainExtent)
{
	if(!windowResolutionDependant) return;

	if(info.windowResolutionRatio.x > 0.0f)
		info.imageResolution.x = static_cast<uint32_t>(info.windowResolutionRatio.x * swapchainExtent.width);
	if(info.windowResolutionRatio.y > 0.0f)
		info.imageResolution.y = static_cast<uint32_t>(info.windowResolutionRatio.y * swapchainExtent.height);

	pushConstantData.imagesSize = info.imageResolution;
	if(info.calculateWorkgroupsFromImage[0])
		info.workgroups.x = (info.imageResolution.x + info.workgroupSize.x - 1U) / info.workgroupSize.x;
	if(info.calculateWorkgroupsFromImage[1])
		info.workgroups.y = (info.imageResolution.y + info.workgroupSize.y - 1U) / info.workgroupSize.y;

	pushConstantData.iterationCounter = 0U;

	outputImage.resize
	(
		mtdDevice,
		info.imageResolution,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::e2D
	);

	for(Image& image: images)
	{
		image.resize
		(
			mtdDevice,
			info.imageResolution,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::ImageAspectFlagBits::eColor,
			vk::ImageViewType::e2D
		);
	}
	configurePipelineDescriptorSet();
}

void mtd::ComputePipeline::loadShaderModule()
{
	shaders.emplace_back(device, vk::ShaderStageFlagBits::eCompute, info.computeShaderPath.c_str());
}

void mtd::ComputePipeline::createDescriptorSetLayouts()
{
	descriptorSetHandlers.reserve(info.descriptorSetInfo.size() == 0 ? 1 : 2);

	uint32_t bindingIndex = 0U;

	std::vector<vk::DescriptorSetLayoutBinding> bindings(3);
	while(bindingIndex < bindings.size())
	{
		bindings[bindingIndex].binding = bindingIndex;
		bindings[bindingIndex].descriptorType = vk::DescriptorType::eStorageImage;
		bindings[bindingIndex].descriptorCount = 1;
		bindings[bindingIndex].stageFlags = vk::ShaderStageFlagBits::eCompute;
		bindings[bindingIndex].pImmutableSamplers = nullptr;
		bindingIndex++;
	}

	descriptorSetHandlers.emplace_back(device, bindings);
	descriptorTypeCount[vk::DescriptorType::eStorageImage] += 3U;

	if(info.descriptorSetInfo.size() == 0) return;
	bindings.clear();

	DescriptorSetBuilder::buildDescriptorSetLayout(bindings, info.descriptorSetInfo, descriptorTypeCount);
	descriptorSetHandlers.emplace_back(device, bindings);
}

void mtd::ComputePipeline::createPipelineLayout(const vk::DescriptorSetLayout& globalDescriptorSetLayout)
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{globalDescriptorSetLayout};
	for(const DescriptorSetHandler& descriptorSetHandler: descriptorSetHandlers)
		descriptorSetLayouts.push_back(descriptorSetHandler.getLayout());

	vk::PushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(ComputeShaderPushConstantData);

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.flags = vk::PipelineLayoutCreateFlags();
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	vk::Result result = device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create compute pipeline layout. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created compute pipeline layout.");
}

void mtd::ComputePipeline::createComputePipeline()
{
	assert(shaders.size() == 1 && "There must be one compute shader module loaded to create the compute pipeline.");

	vk::ComputePipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.flags = vk::PipelineCreateFlags();
	pipelineCreateInfo.stage = shaders[0].generatePipelineShaderCreateInfo();
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = 0;

	vk::Result result = device.createComputePipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create compute pipeline. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created compute pipeline.");
}

void mtd::ComputePipeline::createStorageImages(const Device& mtdDevice, vk::Extent2D swapchainExtent)
{
	images.reserve(2);
	images.emplace_back(mtdDevice.getDevice());
	images.emplace_back(mtdDevice.getDevice());

	if(info.windowResolutionRatio.x > 0.0f)
	{
		info.imageResolution.x = static_cast<uint32_t>(info.windowResolutionRatio.x * swapchainExtent.width);
		windowResolutionDependant = true;
	}
	if(info.windowResolutionRatio.y > 0.0f)
	{
		info.imageResolution.y = static_cast<uint32_t>(info.windowResolutionRatio.y * swapchainExtent.height);
		windowResolutionDependant = true;
	}

	pushConstantData.imagesSize = info.imageResolution;
	if(info.calculateWorkgroupsFromImage[0])
		info.workgroups.x = (info.imageResolution.x + info.workgroupSize.x - 1U) / info.workgroupSize.x;
	if(info.calculateWorkgroupsFromImage[1])
		info.workgroups.y = (info.imageResolution.y + info.workgroupSize.y - 1U) / info.workgroupSize.y;

	outputImage.createImage
	(
		info.imageResolution,
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled
	);
	outputImage.createImageMemory(mtdDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
	outputImage.createImageView(vk::ImageAspectFlagBits::eColor, vk::ImageViewType::e2D);
	outputImage.createImageSampler(vk::Filter::eNearest);

	for(Image& image: images)
	{
		image.createImage
		(
			info.imageResolution,
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled
		);
		image.createImageMemory(mtdDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
		image.createImageView(vk::ImageAspectFlagBits::eColor, vk::ImageViewType::e2D);
		image.createImageSampler(vk::Filter::eLinear);
	}
}
