#include <pch.hpp>
#include "RayTracingPipeline.hpp"

#include "Builders/DescriptorSetBuilder.hpp"
#include "../../Utils/Logger.hpp"

static constexpr uint32_t MAX_TEXTURE_COUNT = 1024U;

mtd::RayTracingPipeline::RayTracingPipeline
(
	const Device& mtdDevice,
	const RayTracingPipelineInfo& info,
	const vk::DescriptorSetLayout& globalDescriptorSetLayout,
	vk::Extent2D swapchainExtent
) : Pipeline{mtdDevice.getDevice(), info},
	outputImage{mtdDevice.getDevice()},
	accumulationImage{mtdDevice.getDevice()},
	windowResolutionDependant{false},
	sbtBuffer
	{
		mtdDevice,
		vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress |
			vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
	},
	shaderRenderingInfo{2U, 4U, 1U, 0U, 0U}
{
	loadShaderModules();
	createDescriptorSetLayouts();
	createPipelineLayout(globalDescriptorSetLayout);
	createRayTracingPipeline(mtdDevice.getDLDI());
	createShaderBindingTable(mtdDevice);
	createStorageImages(mtdDevice, swapchainExtent);
	setEventCallback();
}

mtd::RayTracingPipeline::RayTracingPipeline(RayTracingPipeline&& other) noexcept
	: Pipeline{std::move(other)},
	outputImage{std::move(other.outputImage)},
	accumulationImage{std::move(other.accumulationImage)},
	windowResolutionDependant{other.windowResolutionDependant},
	sbtBuffer{std::move(other.sbtBuffer)},
	rayGenRegionSBT{std::move(other.rayGenRegionSBT)},
	missRegionSBT{std::move(other.missRegionSBT)},
	hitRegionSBT{std::move(other.hitRegionSBT)},
	callableRegionSBT{std::move(other.callableRegionSBT)}
{}

void mtd::RayTracingPipeline::traceRays
(
	const vk::CommandBuffer& commandBuffer, const vk::detail::DispatchLoaderDynamic& dldi
) const
{
	outputImage.transitionImageLayout(commandBuffer, vk::ImageLayout::eGeneral);
	accumulationImage.transitionImageLayout(commandBuffer, vk::ImageLayout::eGeneral);

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

	shaderRenderingInfo.randomSeed = rand();
	commandBuffer.pushConstants
	(
		pipelineLayout,
		vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR,
		0, sizeof(RayTracingRenderData), &shaderRenderingInfo
	);

	commandBuffer.traceRaysKHR
	(
		rayGenRegionSBT,
		missRegionSBT,
		hitRegionSBT,
		callableRegionSBT,
		info.width, info.height, 2,
		dldi
	);

	outputImage.transitionImageLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);

	shaderRenderingInfo.accumulatedFrames++;
}

void mtd::RayTracingPipeline::configurePipelineDescriptorSet()
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	outputImage.defineDescriptorImageInfo(&descriptorImageInfo);
	descriptorImageInfo.imageLayout = vk::ImageLayout::eGeneral;
	descriptorSetHandlers[0].createImageDescriptorResources(0, 1, descriptorImageInfo);

	accumulationImage.defineDescriptorImageInfo(&descriptorImageInfo);
	descriptorImageInfo.imageLayout = vk::ImageLayout::eGeneral;
	descriptorSetHandlers[0].createImageDescriptorResources(0, 2, descriptorImageInfo);

	descriptorSetHandlers[0].writeDescriptorSet(0);
}

void mtd::RayTracingPipeline::shareRenderTargetImageDescriptor
(
	DescriptorSetHandler& descriptorSetHandler, uint32_t binding
) const
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	outputImage.defineDescriptorImageInfo(&descriptorImageInfo);
	descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	descriptorSetHandler.createImageDescriptorResources(0, binding, descriptorImageInfo);
}

void mtd::RayTracingPipeline::resize(const Device& mtdDevice, vk::Extent2D swapchainExtent)
{
	if(!windowResolutionDependant) return;

	if(info.windowResolutionRatio.x > 0.0f)
		info.width = static_cast<uint32_t>(info.windowResolutionRatio.x * swapchainExtent.width);
	if(info.windowResolutionRatio.y > 0.0f)
		info.height = static_cast<uint32_t>(info.windowResolutionRatio.y * swapchainExtent.height);

	outputImage.resize
	(
		mtdDevice,
		{info.width, info.height},
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::e2D
	);
	accumulationImage.resize
	(
		mtdDevice,
		{info.width, info.height},
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eStorage,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::e2D
	);
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
	descriptorSetHandlers.reserve(info.descriptorSetInfo.size() == 0 ? 1 : 2);

	const uint32_t bindingCount = info.materialTextureTypes.empty() ? 7U : 8U;
	uint32_t bindingIndex = 0U;
	std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(bindingCount);

	layoutBindings[bindingIndex].binding = bindingIndex;
	layoutBindings[bindingIndex].descriptorType = vk::DescriptorType::eAccelerationStructureKHR;
	layoutBindings[bindingIndex].descriptorCount = 1U;
	layoutBindings[bindingIndex].stageFlags =
		vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR;
	layoutBindings[bindingIndex].pImmutableSamplers = nullptr;
	bindingIndex++;

	layoutBindings[bindingIndex].binding = bindingIndex;
	layoutBindings[bindingIndex].descriptorType = vk::DescriptorType::eStorageImage;
	layoutBindings[bindingIndex].descriptorCount = 1U;
	layoutBindings[bindingIndex].stageFlags = vk::ShaderStageFlagBits::eRaygenKHR;
	layoutBindings[bindingIndex].pImmutableSamplers = nullptr;
	bindingIndex++;

	layoutBindings[bindingIndex].binding = bindingIndex;
	layoutBindings[bindingIndex].descriptorType = vk::DescriptorType::eStorageImage;
	layoutBindings[bindingIndex].descriptorCount = 1U;
	layoutBindings[bindingIndex].stageFlags = vk::ShaderStageFlagBits::eRaygenKHR;
	layoutBindings[bindingIndex].pImmutableSamplers = nullptr;
	bindingIndex++;

	while(bindingIndex < 7U)
	{
		layoutBindings[bindingIndex].binding = bindingIndex;
		layoutBindings[bindingIndex].descriptorType = vk::DescriptorType::eStorageBuffer;
		layoutBindings[bindingIndex].descriptorCount = 1U;
		layoutBindings[bindingIndex].stageFlags = vk::ShaderStageFlagBits::eClosestHitKHR;
		layoutBindings[bindingIndex].pImmutableSamplers = nullptr;

		bindingIndex++;
	}

	vk::DescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingsCreateInfo{};
	const vk::DescriptorSetLayoutBindingFlagsCreateInfo* pDescriptorSetLayoutBindingsCreateInfo = nullptr;
	std::array<vk::DescriptorBindingFlags, 8> bindingFlags;
	if(!info.materialTextureTypes.empty())
	{
		layoutBindings[bindingIndex].binding = bindingIndex;
		layoutBindings[bindingIndex].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		layoutBindings[bindingIndex].descriptorCount = MAX_TEXTURE_COUNT;
		layoutBindings[bindingIndex].stageFlags = vk::ShaderStageFlagBits::eClosestHitKHR;
		layoutBindings[bindingIndex].pImmutableSamplers = nullptr;

		bindingFlags.back() = vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
			vk::DescriptorBindingFlagBits::ePartiallyBound;
		
		descriptorSetLayoutBindingsCreateInfo.bindingCount = bindingCount;
		descriptorSetLayoutBindingsCreateInfo.pBindingFlags = bindingFlags.data();
		pDescriptorSetLayoutBindingsCreateInfo = &descriptorSetLayoutBindingsCreateInfo;
	}

	descriptorSetHandlers.emplace_back(device, layoutBindings, pDescriptorSetLayoutBindingsCreateInfo);

	if(info.descriptorSetInfo.size() == 0) return;
	layoutBindings.clear();

	DescriptorSetBuilder::buildDescriptorSetLayout(layoutBindings, info.descriptorSetInfo, descriptorTypeCount);
	descriptorSetHandlers.emplace_back(device, layoutBindings);
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

	outputImage.createImage
	(
		{info.width, info.height},
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled
	);
	outputImage.createImageMemory(mtdDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
	outputImage.createImageView(vk::ImageAspectFlagBits::eColor, vk::ImageViewType::e2D);
	outputImage.createImageSampler(vk::Filter::eLinear);

	accumulationImage.createImage
	(
		{info.width, info.height},
		vk::Format::eR32G32B32A32Sfloat,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eStorage
	);
	accumulationImage.createImageMemory(mtdDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
	accumulationImage.createImageView(vk::ImageAspectFlagBits::eColor, vk::ImageViewType::e2D);
	accumulationImage.createImageSampler(vk::Filter::eLinear);
}

void mtd::RayTracingPipeline::createPipelineLayout(const vk::DescriptorSetLayout& globalDescriptorSetLayout)
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{globalDescriptorSetLayout};
	for(const DescriptorSetHandler& descriptorSetHandler: descriptorSetHandlers)
		descriptorSetLayouts.push_back(descriptorSetHandler.getLayout());

	vk::PushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(RayTracingRenderData);

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.flags = vk::PipelineLayoutCreateFlags();
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
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
		device.createRayTracingPipelinesKHR(nullptr, nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline, dldi);
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

	const uint32_t handleSizeAligned = (handleSize + handleAlignment - 1) & ~(handleAlignment - 1);
	const uint32_t baseSizeAligned = (handleSize + baseAlignment - 1) & ~(baseAlignment - 1);
	
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
	sbtBuffer.create(sbtSize);
	vk::DeviceAddress sbtAddress = sbtBuffer.getBufferAddress();

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
	vk::DeviceSize bufferOffset = 0;

	sbtBuffer.copyMemoryToBuffer(handleSize, pShaderHandleStorage, bufferOffset);
	pShaderHandleStorage += handleSize;
	bufferOffset += rayGenRegionSBT.size;

	sbtBuffer.copyMemoryToBuffer(handleSize, pShaderHandleStorage, bufferOffset);
	pShaderHandleStorage += handleSize;
	bufferOffset += missRegionSBT.size;

	sbtBuffer.copyMemoryToBuffer(handleSize, pShaderHandleStorage, bufferOffset);
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

	shaderGroupCreateInfos[2].type = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
	shaderGroupCreateInfos[2].generalShader = vk::ShaderUnusedKHR;
	shaderGroupCreateInfos[2].closestHitShader = 2;
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
