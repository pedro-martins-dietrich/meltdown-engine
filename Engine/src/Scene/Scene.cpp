#include <pch.hpp>
#include "Scene.hpp"

#include "SceneLoader.hpp"
#include "../Utils/Logger.hpp"

mtd::Scene::Scene(const Device& mtdDevice) : instanceManager{}, descriptorPool{mtdDevice.getDevice()}
{}

void mtd::Scene::loadScene
(
	const Device& mtdDevice,
	std::string_view sceneFileName,
	std::vector<FramebufferInfo>& framebufferInfos,
	PipelineInfoBundle& pipelineInfos,
	ResourceManager& resourceManager,
	DescriptorManager& descriptorManager,
	std::vector<RenderPassInfo>& renderOrder
)
{
	renderOrder.clear();
	if(rayTracingBVH.has_value())
		rayTracingBVH.value().clear();

	SceneLoader::load
	(
		mtdDevice,
		sceneFileName,
		framebufferInfos,
		pipelineInfos,
		renderOrder,
		meshes,
		resourceManager,
		descriptorManager,
		instanceManager,
		rayTracingBVH,
		gpuResources
	);
}

void mtd::Scene::allocateResources(PipelineBundle& pipelines)
{
	descriptorPool.clear();

	std::unordered_map<vk::DescriptorType, uint32_t> totalDescriptorTypeCount;
	totalDescriptorTypeCount[vk::DescriptorType::eUniformBuffer] = 1U;
	totalDescriptorTypeCount[vk::DescriptorType::eStorageBuffer] = 7U;

	uint32_t totalImageSamplerCount = gpuResources.textureIDs.size() + 1U;
	for(const FramebufferPipeline& fbPipeline: pipelines.framebufferPipelines)
		totalImageSamplerCount += fbPipeline.getImageDescriptorsCount();
	if(totalImageSamplerCount > 0U)
		totalDescriptorTypeCount[vk::DescriptorType::eCombinedImageSampler] = totalImageSamplerCount;

	for(const RasterizationPipeline& rasterPipeline: pipelines.rasterizationPipelines)
		for(const auto& [type, count]: rasterPipeline.getDescriptorTypeCount())
			totalDescriptorTypeCount[type] += count;
	for(const FramebufferPipeline& fbPipeline: pipelines.framebufferPipelines)
		for(const auto& [type, count]: fbPipeline.getDescriptorTypeCount())
			totalDescriptorTypeCount[type] += count;
	for(const ComputePipeline& computePipeline: pipelines.computePipelines)
		for(const auto& [type, count]: computePipeline.getDescriptorTypeCount())
			totalDescriptorTypeCount[type] += count;
	for(const RayTracingPipeline& rtPipeline: pipelines.rayTracingPipelines)
		for(const auto& [type, count]: rtPipeline.getDescriptorTypeCount())
			totalDescriptorTypeCount[type] += count;

	std::vector<PoolSizeData> poolSizesInfo{totalDescriptorTypeCount.size()};
	uint32_t i = 0U;
	for(const auto& [type, count]: totalDescriptorTypeCount)
	{
		poolSizesInfo[i].descriptorCount = count;
		poolSizesInfo[i].descriptorType = type;
		i++;
	}

	descriptorPool.createDescriptorPool(poolSizesInfo, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

	for(FramebufferPipeline& fbPipeline: pipelines.framebufferPipelines)
	{
		DescriptorSetHandler& descriptorSetHandler = fbPipeline.getDescriptorSetHandler(0U);
		descriptorPool.allocateDescriptorSet(descriptorSetHandler);
	}
	for(ComputePipeline& computePipeline: pipelines.computePipelines)
	{
		DescriptorSetHandler& descriptorSetHandler = computePipeline.getDescriptorSetHandler(0U);
		descriptorPool.allocateDescriptorSet(descriptorSetHandler);
	}
	for(uint32_t i = 0U; i < pipelines.rayTracingPipelines.size(); i++)
	{
		DescriptorSetHandler& descriptorSetHandler = pipelines.rayTracingPipelines[i].getDescriptorSetHandler(0U);

		totalImageSamplerCount--;

		vk::DescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
		variableCountInfo.descriptorSetCount = 1U;
		variableCountInfo.pDescriptorCounts = &totalImageSamplerCount;
		descriptorPool.allocateDescriptorSet(descriptorSetHandler, &variableCountInfo);
	}
	LOG_INFO("Meshes loaded to the GPU.\n");
}

void mtd::Scene::bindMeshData(const ResourceManager& resourceManager, vk::CommandBuffer commandBuffer) const
{
	vk::DeviceSize offset{0UL};
	vk::Buffer vertexBuffer = resourceManager.getVulkanBuffer(gpuResources.vertexBufferID);
	vk::Buffer indexBuffer = resourceManager.getVulkanBuffer(gpuResources.indexBufferID);

	if(vertexBuffer)
		commandBuffer.bindVertexBuffers(0U, 1U, &vertexBuffer, &offset);
	if(indexBuffer)
		commandBuffer.bindIndexBuffer(indexBuffer, offset, vk::IndexType::eUint32);
}

void mtd::Scene::start() const
{}

void mtd::Scene::update(double frameTime) const
{}
