#include <pch.hpp>
#include "RayTracingPipeline.hpp"

#include "Builders/DescriptorSetBuilder.hpp"
#include "Builders/PipelineMapping.hpp"
#include "../../Utils/Logger.hpp"

mtd::RayTracingPipeline::RayTracingPipeline
(
	const Device& mtdDevice,
	const RayTracingPipelineInfo& info,
	const vk::DescriptorSetLayout& globalDescriptorSetLayout,
	vk::Extent2D swapchainExtent
) : Pipeline{mtdDevice.getDevice(), info},
	image{mtdDevice.getDevice()},
	windowResolutionDependant{false},
	sbtBuffer
	{
		mtdDevice,
		vk::BufferUsageFlagBits::eShaderBindingTableKHR |
			vk::BufferUsageFlagBits::eShaderDeviceAddress |
			vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
	}
{
	loadShaderModules();
	createDescriptorSetLayouts();
	createPipelineLayout(globalDescriptorSetLayout);
	createRayTracingPipeline(mtdDevice.getDLDI());
	createShaderBindingTable(mtdDevice);
	createStorageImage(mtdDevice, swapchainExtent);
}

mtd::RayTracingPipeline::RayTracingPipeline(RayTracingPipeline&& other) noexcept
	: Pipeline{std::move(other)},
	image{std::move(other.image)},
	windowResolutionDependant{other.windowResolutionDependant},
	sbtBuffer{std::move(other.sbtBuffer)},
	rayGenRegionSBT{std::move(other.rayGenRegionSBT)},
	missRegionSBT{std::move(other.missRegionSBT)},
	hitRegionSBT{std::move(other.hitRegionSBT)},
	callableRegionSBT{std::move(other.callableRegionSBT)}
{}

// Binds the pipeline and performs the ray tracing
void mtd::RayTracingPipeline::traceRays
(
	const vk::CommandBuffer& commandBuffer, const vk::detail::DispatchLoaderDynamic& dldi
) const
{
	image.transitionImageLayout(commandBuffer, vk::ImageLayout::eGeneral);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline);
	commandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eRayTracingKHR,
		pipelineLayout,
		1,
		1, &(descriptorSetHandlers[0].getSet(0)),
		0, nullptr
	);

	if(descriptorSetHandlers.size() > 1 && descriptorSetHandlers[1].getSetCount() > 0)
	{
		commandBuffer.bindDescriptorSets
		(
			vk::PipelineBindPoint::eRayTracingKHR,
			pipelineLayout,
			2,
			1, &(descriptorSetHandlers[1].getSet(0)),
			0, nullptr
		);
	}

	commandBuffer.traceRaysKHR
	(
		rayGenRegionSBT,
		missRegionSBT,
		hitRegionSBT,
		callableRegionSBT,
		info.width, info.height, 1,
		dldi
	);

	image.transitionImageLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);
}

// Configures the render target image descriptor
void mtd::RayTracingPipeline::configurePipelineDescriptorSet()
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	image.defineDescriptorImageInfo(&descriptorImageInfo);
	descriptorImageInfo.imageLayout = vk::ImageLayout::eGeneral;

	descriptorSetHandlers[0].createImageDescriptorResources(0, 0, descriptorImageInfo);
	descriptorSetHandlers[0].writeDescriptorSet(0);
}

// Configures the render target image as a descriptor for another descriptor set
void mtd::RayTracingPipeline::shareRenderTargetImageDescriptor
(
	DescriptorSetHandler& descriptorSetHandler, uint32_t binding
) const
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	image.defineDescriptorImageInfo(&descriptorImageInfo);
	descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	descriptorSetHandler.createImageDescriptorResources(0, binding, descriptorImageInfo);
}

// Resizes the render target image if needed
void mtd::RayTracingPipeline::resize(const Device& mtdDevice, vk::Extent2D swapchainExtent)
{
	if(!windowResolutionDependant) return;

	if(info.windowResolutionRatio.x > 0.0f)
		info.width = static_cast<uint32_t>(info.windowResolutionRatio.x * swapchainExtent.width);
	if(info.windowResolutionRatio.y > 0.0f)
		info.height = static_cast<uint32_t>(info.windowResolutionRatio.y * swapchainExtent.height);

	image.resize
	(
		mtdDevice,
		{info.width, info.height},
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::e2D
	);
	configurePipelineDescriptorSet();
}

// Loads the pipeline shader modules
void mtd::RayTracingPipeline::loadShaderModules()
{
	shaders.reserve(2);
	shaders.emplace_back(device, vk::ShaderStageFlagBits::eRaygenKHR, info.rayGenShaderPath.c_str());
	shaders.emplace_back(device, vk::ShaderStageFlagBits::eMissKHR, info.missShaderPath.c_str());
}

// Configures the descriptor set handlers to be used
void mtd::RayTracingPipeline::createDescriptorSetLayouts()
{
	descriptorSetHandlers.reserve(info.descriptorSetInfo.size() == 0 ? 1 : 2);

	const uint32_t bindingCount = 5U;
	std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(bindingCount);
	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorType = vk::DescriptorType::eStorageImage;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eMissKHR;
	layoutBindings[0].pImmutableSamplers = nullptr;

	for(uint32_t i = 1; i < bindingCount; i++)
	{
		layoutBindings[i].binding = i;
		layoutBindings[i].descriptorType = vk::DescriptorType::eStorageBuffer;
		layoutBindings[i].descriptorCount = 1;
		layoutBindings[i].stageFlags = vk::ShaderStageFlagBits::eRaygenKHR;
		layoutBindings[i].pImmutableSamplers = nullptr;
	}

	descriptorSetHandlers.emplace_back(device, layoutBindings);

	if(info.descriptorSetInfo.size() == 0) return;
	layoutBindings.clear();

	DescriptorSetBuilder::buildDescriptorSetLayout(layoutBindings, info.descriptorSetInfo, descriptorTypeCount);
	descriptorSetHandlers.emplace_back(device, layoutBindings);
}

// Creates the storage image for the ray trace rendering
void mtd::RayTracingPipeline::createStorageImage(const Device& mtdDevice, vk::Extent2D swapchainExtent)
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

	image.createImage
	(
		{info.width, info.height},
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled
	);
	image.createImageMemory(mtdDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
	image.createImageView(vk::ImageAspectFlagBits::eColor, vk::ImageViewType::e2D);
	image.createImageSampler(vk::Filter::eLinear);
}

// Creates the layout for the pipeline
void mtd::RayTracingPipeline::createPipelineLayout(const vk::DescriptorSetLayout& globalDescriptorSetLayout)
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
		LOG_ERROR("Failed to create ray tracing pipeline layout. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created ray tracing pipeline layout.");
}

// Creates the Vulkan ray tracing pipeline
void mtd::RayTracingPipeline::createRayTracingPipeline(const vk::detail::DispatchLoaderDynamic& dldi)
{
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;
	shaderStageInfos.reserve(shaders.size());
	for(const ShaderModule& shader: shaders)
		shaderStageInfos.emplace_back(shader.generatePipelineShaderCreateInfo());

	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroupCreateInfos;
	defineShaderGroups(shaderGroupCreateInfos);

	vk::RayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
	pipelineCreateInfo.flags = vk::PipelineCreateFlags();
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStageInfos.size());
	pipelineCreateInfo.pStages = shaderStageInfos.data();
	pipelineCreateInfo.groupCount = static_cast<uint32_t>(shaderGroupCreateInfos.size());
	pipelineCreateInfo.pGroups = shaderGroupCreateInfos.data();
	pipelineCreateInfo.maxPipelineRayRecursionDepth = 1;
	pipelineCreateInfo.pLibraryInfo = nullptr;
	pipelineCreateInfo.pLibraryInterface = nullptr;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = 0;

	vk::Result result =
		device.createRayTracingPipelinesKHR(nullptr, nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline, dldi);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create ray tracing pipeline. Vulkan result: %d", result);
		return;
	}
	LOG_INFO("Created ray tracing pipeline.\n");
}

// Creates the Shader Binding Table (SBT) and the regions
void mtd::RayTracingPipeline::createShaderBindingTable(const Device& mtdDevice)
{
	const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& rayTracingProperties =
		mtdDevice.fetchRayTracingProperties();
	const uint32_t shaderGroupHandleSize =
		(rayTracingProperties.shaderGroupHandleSize + rayTracingProperties.shaderGroupBaseAlignment - 1) &
		~(rayTracingProperties.shaderGroupBaseAlignment - 1);
	const vk::DeviceSize sbtSize = shaders.size() * shaderGroupHandleSize;

	sbtBuffer.create(sbtSize);

	std::vector<uint8_t> shaderHandleStorage(sbtSize);
	vk::Result result = device.getRayTracingShaderGroupHandlesKHR
	(
		pipeline, 0U, static_cast<uint32_t>(shaders.size()), sbtSize, shaderHandleStorage.data(), mtdDevice.getDLDI()
	);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to get ray tracing shader group handle. Vulkan result: %d", result);
		return;
	}

	sbtBuffer.copyMemoryToBuffer(sbtSize, shaderHandleStorage.data());

	vk::BufferDeviceAddressInfo bufferAddressInfo{sbtBuffer.getBuffer()};
	vk::DeviceAddress sbtAddress = device.getBufferAddress(bufferAddressInfo);

	rayGenRegionSBT.deviceAddress = sbtAddress;
	rayGenRegionSBT.stride = shaderGroupHandleSize;
	rayGenRegionSBT.size = shaderGroupHandleSize;

	missRegionSBT.deviceAddress = sbtAddress + static_cast<vk::DeviceSize>(shaderGroupHandleSize);
	missRegionSBT.stride = shaderGroupHandleSize;
	missRegionSBT.size = shaderGroupHandleSize;
}

// Defines the shader groups
void mtd::RayTracingPipeline::defineShaderGroups
(
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR>& shaderGroupCreateInfos
) const
{
	shaderGroupCreateInfos.resize(2);
	shaderGroupCreateInfos[0].type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
	shaderGroupCreateInfos[0].generalShader = 0;
	shaderGroupCreateInfos[0].closestHitShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[0].anyHitShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[0].intersectionShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[0].pShaderGroupCaptureReplayHandle = nullptr;

	shaderGroupCreateInfos[1].type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
	shaderGroupCreateInfos[1].generalShader = 1;
	shaderGroupCreateInfos[1].closestHitShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[1].anyHitShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[1].intersectionShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[1].pShaderGroupCaptureReplayHandle = nullptr;
}
