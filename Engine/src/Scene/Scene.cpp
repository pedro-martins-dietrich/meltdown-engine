#include <pch.hpp>
#include "Scene.hpp"

#include "SceneLoader.hpp"
#include "../Utils/Logger.hpp"
#include "../Vulkan/Mesh/MeshManager.hpp"

mtd::Scene::Scene(const Device& device) : descriptorPool{device.getDevice()}
{}

void mtd::Scene::loadScene
(
	const Device& device,
	const char* sceneFileName,
	std::vector<FramebufferInfo>& framebufferInfos,
	PipelineInfoBundle& pipelineInfos,
	std::vector<RenderPassInfo>& renderOrder
)
{
	renderOrder.clear();
	meshManagers.clear();

	SceneLoader::load
	(
		device,
		sceneFileName,
		framebufferInfos,
		pipelineInfos,
		renderOrder,
		meshManagers
	);
}

void mtd::Scene::allocateResources(PipelineBundle& pipelines)
{
	descriptorPool.clear();

	std::unordered_map<vk::DescriptorType, uint32_t> totalDescriptorTypeCount;
	totalDescriptorTypeCount[vk::DescriptorType::eUniformBuffer] = 1 + getMaterialFloatDataCount();

	uint32_t totalImageSamplerCount = getTotalTextureCount();
	for(const FramebufferPipeline& fbPipeline: pipelines.framebufferPipelines)
		totalImageSamplerCount += fbPipeline.getImageDescriptorsCount();
	if(totalImageSamplerCount > 0)
		totalDescriptorTypeCount[vk::DescriptorType::eCombinedImageSampler] = totalImageSamplerCount;

	for(const GraphicsPipeline& rasterPipeline: pipelines.graphicsPipelines)
		for(const auto& [type, count]: rasterPipeline.getDescriptorTypeCount())
			totalDescriptorTypeCount[type] += count;
	for(const FramebufferPipeline& fbPipeline: pipelines.framebufferPipelines)
		for(const auto& [type, count]: fbPipeline.getDescriptorTypeCount())
			totalDescriptorTypeCount[type] += count;
	for(const ComputePipeline& computePipeline: pipelines.computePipelines)
		for(const auto& [type, count]: computePipeline.getDescriptorTypeCount())
			totalDescriptorTypeCount[type] += count;
	for(const RayTracingPipeline& rtPipeline: pipelines.rayTracingPipelines)
	{
		for(const auto& [type, count]: rtPipeline.getDescriptorTypeCount())
			totalDescriptorTypeCount[type] += count;
		totalDescriptorTypeCount[vk::DescriptorType::eAccelerationStructureKHR]++;
		totalDescriptorTypeCount[vk::DescriptorType::eStorageImage] += 2U;
		totalDescriptorTypeCount[vk::DescriptorType::eStorageBuffer] += 4U;
	}

	std::vector<PoolSizeData> poolSizesInfo{totalDescriptorTypeCount.size()};
	uint32_t i = 0;
	for(const auto& [type, count]: totalDescriptorTypeCount)
	{
		poolSizesInfo[i].descriptorCount = count;
		poolSizesInfo[i].descriptorType = type;
		i++;
	}

	descriptorPool.createDescriptorPool(poolSizesInfo, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

	for(uint32_t i = 0; i < pipelines.graphicsPipelines.size(); i++)
	{
		const std::unique_ptr<MeshManager>& pMeshManager = meshManagers[i];
		if(pMeshManager->getMeshCount() == 0) continue;

		DescriptorSetHandler& descriptorSetHandler = pipelines.graphicsPipelines[i].getDescriptorSetHandler(0);
		descriptorSetHandler.defineDescriptorSetsAmount(pMeshManager->getMaterialCount());
		descriptorPool.allocateDescriptorSet(descriptorSetHandler);

		pMeshManager->loadMeshes(descriptorSetHandler);
	}
	for(FramebufferPipeline& fbPipeline: pipelines.framebufferPipelines)
	{
		DescriptorSetHandler& descriptorSetHandler = fbPipeline.getDescriptorSetHandler(0);
		descriptorSetHandler.defineDescriptorSetsAmount(1);
		descriptorPool.allocateDescriptorSet(descriptorSetHandler);
	}
	for(ComputePipeline& computePipeline: pipelines.computePipelines)
	{
		DescriptorSetHandler& descriptorSetHandler = computePipeline.getDescriptorSetHandler(0);
		descriptorSetHandler.defineDescriptorSetsAmount(1);
		descriptorPool.allocateDescriptorSet(descriptorSetHandler);
	}
	for(uint32_t i = 0; i < pipelines.rayTracingPipelines.size(); i++)
	{
		DescriptorSetHandler& descriptorSetHandler = pipelines.rayTracingPipelines[i].getDescriptorSetHandler(0);
		descriptorSetHandler.defineDescriptorSetsAmount(1);

		vk::DescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
		variableCountInfo.descriptorSetCount = 1U;
		variableCountInfo.pDescriptorCounts = &totalImageSamplerCount;
		descriptorPool.allocateDescriptorSet(descriptorSetHandler, &variableCountInfo);

		meshManagers[pipelines.graphicsPipelines.size() + i]->loadMeshes(descriptorSetHandler);
	}
	LOG_INFO("Meshes loaded to the GPU.\n");
}

void mtd::Scene::start() const
{
	for(const std::unique_ptr<MeshManager>& pMeshManager: meshManagers)
	{
		if(pMeshManager->getMeshCount() > 0)
			pMeshManager->start();
	}
}

void mtd::Scene::update(double frameTime) const
{
	for(const std::unique_ptr<MeshManager>& pMeshManager: meshManagers)
	{
		if(pMeshManager->getMeshCount() > 0)
			pMeshManager->update(frameTime);
	}
}

uint32_t mtd::Scene::getTotalTextureCount() const
{
	uint32_t count = 0;
	for(const std::unique_ptr<MeshManager>& pMeshManager: meshManagers)
		count += pMeshManager->getTextureCount();

	return count;
}

uint32_t mtd::Scene::getMaterialFloatDataCount() const
{
	uint32_t count = 0;
	for(const std::unique_ptr<MeshManager>& pMeshManager: meshManagers)
	{
		if(pMeshManager->hasMaterialFloatData())
			count += pMeshManager->getMaterialCount();
	}
	return count;
}
