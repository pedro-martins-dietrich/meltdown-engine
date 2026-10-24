#include <pch.hpp>
#include "RayTracingPipeline.hpp"

#include "../../Utils/Logger.hpp"

mtd::RayTracingPipeline::RayTracingPipeline
(
	const Device& mtdDevice,
	ResourceManager& resourceManager,
	const DescriptorManager& descriptorManager,
	const RayTracingPipelineInfo& info,
	vk::Extent2D swapchainExtent
) : Pipeline{mtdDevice.getDevice(), descriptorManager, info},
	resourceManager{resourceManager},
	outputImage{mtdDevice},
	accumulationImage{mtdDevice},
	shaderRenderingInfo{2U, 4U, 1U, 0U, 0U}
{
	loadShaderModules();
	createDescriptorSetLayouts();
	createPipelineLayout();
	createRayTracingPipeline(mtdDevice.getDLDI());
	createShaderBindingTable(mtdDevice);
	createStorageImages(mtdDevice, swapchainExtent);
	setEventCallback();
}

mtd::RayTracingPipeline::~RayTracingPipeline()
{
	resourceManager.deleteResource(sbtBufferID);
}

mtd::RayTracingPipeline::RayTracingPipeline(RayTracingPipeline&& other) noexcept
	: Pipeline{std::move(other)},
	resourceManager{other.resourceManager},
	outputImage{std::move(other.outputImage)},
	accumulationImage{std::move(other.accumulationImage)},
	windowResolutionDependant{other.windowResolutionDependant},
	sbtBufferID{other.sbtBufferID},
	rayGenRegionSBT{std::move(other.rayGenRegionSBT)},
	missRegionSBT{std::move(other.missRegionSBT)},
	hitRegionSBT{std::move(other.hitRegionSBT)},
	callableRegionSBT{std::move(other.callableRegionSBT)}
{
	other.sbtBufferID = 0U;
}

void mtd::RayTracingPipeline::traceRays
(
	const vk::CommandBuffer& commandBuffer, const vk::detail::DispatchLoaderDynamic& dldi
) const
{
	outputImage.transitionImageLayout
	(
		commandBuffer, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eRayTracingShaderKHR
	);
	accumulationImage.transitionImageLayout
	(
		commandBuffer, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eRayTracingShaderKHR
	);

	std::vector<vk::DescriptorSet> descriptorSets;
	descriptorSets.reserve(info.descriptorSetIDs.size() + 1);
	for(DescriptorSetID setID: info.descriptorSetIDs)
		descriptorSets.emplace_back(descriptorManager.getSet(setID));
	descriptorSets.emplace_back(descriptorSetHandlers[0].getSet());

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline);
	commandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eRayTracingKHR,
		pipelineLayout,
		0U,
		static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(),
		0U, nullptr
	);

	if(descriptorSetHandlers.size() > 1)
	{
		commandBuffer.bindDescriptorSets
		(
			vk::PipelineBindPoint::eRayTracingKHR,
			pipelineLayout,
			2U,
			1U, &(descriptorSetHandlers[1].getSet()),
			0U, nullptr
		);
	}

	shaderRenderingInfo.randomSeed = rand();
	commandBuffer.pushConstants
	(
		pipelineLayout,
		vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR,
		0U, sizeof(RayTracingRenderData), &shaderRenderingInfo
	);

	commandBuffer.traceRaysKHR
	(
		rayGenRegionSBT,
		missRegionSBT,
		hitRegionSBT,
		callableRegionSBT,
		info.width, info.height, 2U,
		dldi
	);

	outputImage.transitionImageLayout
	(
		commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader
	);

	shaderRenderingInfo.accumulatedFrames++;
}

void mtd::RayTracingPipeline::configurePipelineDescriptorSet()
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	outputImage.updateDescriptorInfo(descriptorImageInfo);
	descriptorImageInfo.imageLayout = vk::ImageLayout::eGeneral;
	descriptorSetHandlers[0].createImageDescriptorResources(0U, descriptorImageInfo);

	accumulationImage.updateDescriptorInfo(descriptorImageInfo);
	descriptorImageInfo.imageLayout = vk::ImageLayout::eGeneral;
	descriptorSetHandlers[0].createImageDescriptorResources(1U, descriptorImageInfo);

	descriptorSetHandlers[0].writeDescriptorSet();
}

void mtd::RayTracingPipeline::shareRenderTargetImageDescriptor
(
	DescriptorSetHandler& descriptorSetHandler, uint32_t binding
) const
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	outputImage.updateDescriptorInfo(descriptorImageInfo);
	descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	descriptorSetHandler.createImageDescriptorResources(binding, descriptorImageInfo);
}

void mtd::RayTracingPipeline::resize(const Device& mtdDevice, vk::Extent2D swapchainExtent)
{
	if(!windowResolutionDependant) return;

	if(info.windowResolutionRatio.x > 0.0f)
		info.width = static_cast<uint32_t>(info.windowResolutionRatio.x * swapchainExtent.width);
	if(info.windowResolutionRatio.y > 0.0f)
		info.height = static_cast<uint32_t>(info.windowResolutionRatio.y * swapchainExtent.height);

	outputImage.resize({info.width, info.height});
	accumulationImage.resize({info.width, info.height});
	configurePipelineDescriptorSet();

	resetAccumulation();
}

void mtd::RayTracingPipeline::loadShaderModules()
{
	shaders.reserve(3);
	shaders.emplace_back(device, vk::ShaderStageFlagBits::eRaygenKHR, info.rayGenShaderPath.c_str());
	shaders.emplace_back(device, vk::ShaderStageFlagBits::eMissKHR, info.missShaderPath.c_str());
	shaders.emplace_back(device, vk::ShaderStageFlagBits::eClosestHitKHR, info.closestHitShaderPath.c_str());
}

void mtd::RayTracingPipeline::createDescriptorSetLayouts()
{
	std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(2);
	layoutBindings[0U].binding = 0U;
	layoutBindings[0U].descriptorType = vk::DescriptorType::eStorageImage;
	layoutBindings[0U].descriptorCount = 1U;
	layoutBindings[0U].stageFlags = vk::ShaderStageFlagBits::eRaygenKHR;
	layoutBindings[0U].pImmutableSamplers = nullptr;

	layoutBindings[1U].binding = 1U;
	layoutBindings[1U].descriptorType = vk::DescriptorType::eStorageImage;
	layoutBindings[1U].descriptorCount = 1U;
	layoutBindings[1U].stageFlags = vk::ShaderStageFlagBits::eRaygenKHR;
	layoutBindings[1U].pImmutableSamplers = nullptr;

	descriptorSetHandlers.emplace_back(device, layoutBindings, nullptr);
	descriptorTypeCount[vk::DescriptorType::eStorageImage] += 2U;
}

void mtd::RayTracingPipeline::createStorageImages(const Device& mtdDevice, vk::Extent2D swapchainExtent)
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

	outputImage.create
	(
		{info.width, info.height},
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::e2D,
		SamplerType::Linear
	);
	accumulationImage.create
	(
		{info.width, info.height},
		vk::Format::eR32G32B32A32Sfloat,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eStorage,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::e2D,
		SamplerType::Linear
	);
}

void mtd::RayTracingPipeline::createPipelineLayout()
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.reserve(info.descriptorLayoutIDs.size() + descriptorSetHandlers.size());
	for(DescriptorLayoutID layoutID: info.descriptorLayoutIDs)
		descriptorSetLayouts.emplace_back(descriptorManager.getLayout(layoutID));
	for(const DescriptorSetHandler& descriptorSetHandler: descriptorSetHandlers)
		descriptorSetLayouts.emplace_back(descriptorSetHandler.getLayout());

	vk::PushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR;
	pushConstantRange.offset = 0U;
	pushConstantRange.size = sizeof(RayTracingRenderData);

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.flags = vk::PipelineLayoutCreateFlags();
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1U;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	vk::Result result = device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create ray tracing pipeline layout. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created ray tracing pipeline layout.");
}

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
	pipelineCreateInfo.maxPipelineRayRecursionDepth = shaderRenderingInfo.maxRecursionDepth;
	pipelineCreateInfo.pLibraryInfo = nullptr;
	pipelineCreateInfo.pLibraryInterface = nullptr;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = 0;

	vk::Result result =
		device.createRayTracingPipelinesKHR(nullptr, nullptr, 1U, &pipelineCreateInfo, nullptr, &pipeline, dldi);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create ray tracing pipeline. Vulkan result: %d", result);
		return;
	}
	LOG_INFO("Created ray tracing pipeline.\n");
}

void mtd::RayTracingPipeline::createShaderBindingTable(const Device& mtdDevice)
{
	const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& rayTracingProperties =
		mtdDevice.fetchRayTracingProperties();

	const uint32_t handleSize = rayTracingProperties.shaderGroupHandleSize;
	const uint32_t handleAlignment = rayTracingProperties.shaderGroupHandleAlignment;
	const uint32_t baseAlignment = rayTracingProperties.shaderGroupBaseAlignment;

	const uint32_t handleSizeAligned = (handleSize + handleAlignment - 1U) & ~(handleAlignment - 1U);
	const uint32_t baseSizeAligned = (handleSize + baseAlignment - 1U) & ~(baseAlignment - 1U);
	
	std::vector<uint8_t> shaderHandleStorage(shaders.size() * handleSize);
	vk::Result result = device.getRayTracingShaderGroupHandlesKHR
	(
		pipeline,
		0U, static_cast<uint32_t>(shaders.size()),
		shaderHandleStorage.size(), shaderHandleStorage.data(),
		mtdDevice.getDLDI()
	);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to get ray tracing shader group handle. Vulkan result: %d", result);
		return;
	}

	const vk::DeviceSize sbtSize = shaders.size() * baseSizeAligned;
	sbtBufferID = resourceManager.createBuffer("", GpuBufferType::ShaderBindingTable, GpuMemoryUsage::CpuUpload, sbtSize);
	const vk::DeviceAddress sbtAddress = resourceManager.getBufferAddress(sbtBufferID);

	rayGenRegionSBT.deviceAddress = sbtAddress;
	rayGenRegionSBT.stride = baseSizeAligned;
	rayGenRegionSBT.size = baseSizeAligned;

	missRegionSBT.deviceAddress = sbtAddress + rayGenRegionSBT.size;
	missRegionSBT.stride = handleSizeAligned;
	missRegionSBT.size = baseSizeAligned;

	hitRegionSBT.deviceAddress = sbtAddress + rayGenRegionSBT.size + missRegionSBT.size;
	hitRegionSBT.stride = handleSizeAligned;
	hitRegionSBT.size = baseSizeAligned;

	uint8_t* pShaderHandleStorage = shaderHandleStorage.data();
	vk::DeviceSize bufferOffset = 0UL;

	resourceManager.updateBufferData(sbtBufferID, handleSize, pShaderHandleStorage, bufferOffset);
	pShaderHandleStorage += handleSize;
	bufferOffset += rayGenRegionSBT.size;

	resourceManager.updateBufferData(sbtBufferID, handleSize, pShaderHandleStorage, bufferOffset);
	pShaderHandleStorage += handleSize;
	bufferOffset += missRegionSBT.size;

	resourceManager.updateBufferData(sbtBufferID, handleSize, pShaderHandleStorage, bufferOffset);
	pShaderHandleStorage += handleSize;
	bufferOffset += hitRegionSBT.size;
}

void mtd::RayTracingPipeline::defineShaderGroups
(
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR>& shaderGroupCreateInfos
) const
{
	shaderGroupCreateInfos.resize(3);
	shaderGroupCreateInfos[0].type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
	shaderGroupCreateInfos[0].generalShader = 0U;
	shaderGroupCreateInfos[0].closestHitShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[0].anyHitShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[0].intersectionShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[0].pShaderGroupCaptureReplayHandle = nullptr;

	shaderGroupCreateInfos[1].type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
	shaderGroupCreateInfos[1].generalShader = 1U;
	shaderGroupCreateInfos[1].closestHitShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[1].anyHitShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[1].intersectionShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[1].pShaderGroupCaptureReplayHandle = nullptr;

	shaderGroupCreateInfos[2].type = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
	shaderGroupCreateInfos[2].generalShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[2].closestHitShader = 2U;
	shaderGroupCreateInfos[2].anyHitShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[2].intersectionShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[2].pShaderGroupCaptureReplayHandle = nullptr;
}

void mtd::RayTracingPipeline::setEventCallback()
{
	resetAccumulationCallbackHandle = EventManager::addCallback([this](const ResetFrameAccumulationEvent& event)
	{
		resetAccumulation();
	});
}
