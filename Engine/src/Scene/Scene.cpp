#include <pch.hpp>
#include "Scene.hpp"

#include "../Utils/Logger.hpp"
#include "../Vulkan/Mesh/MeshManager.hpp"

mtd::Scene::Scene(const Device& device) : descriptorPool{device.getDevice()}
{
}

// Loads scene from file
void mtd::Scene::loadScene
(
	const Device& device,
	const char* sceneFileName,
	std::vector<FramebufferInfo>& framebufferInfos,
	std::vector<PipelineInfo>& pipelineInfos,
	std::vector<FramebufferPipelineInfo>& framebufferPipelineInfos,
	std::vector<RayTracingPipelineInfo>& rayTracingPipelineInfos,
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
		framebufferPipelineInfos,
		rayTracingPipelineInfos,
		renderOrder,
		meshManagers
	);
}

// Allocates resources and loads all mesh data
void mtd::Scene::allocateResources
(
	std::vector<Pipeline>& pipelines,
	std::vector<FramebufferPipeline>& framebufferPipelines,
	std::vector<RayTracingPipeline>& rayTracingPipelines
)
{
	descriptorPool.clear();

	std::unordered_map<vk::DescriptorType, uint32_t> totalDescriptorTypeCount;
	totalDescriptorTypeCount[vk::DescriptorType::eUniformBuffer] = 1 + getMaterialFloatDataCount();

	uint32_t totalImageSamplerCount = getTotalTextureCount();
	for(const FramebufferPipeline& fbPipeline: framebufferPipelines)
		totalImageSamplerCount += fbPipeline.getImageDescriptorsCount();
	if(totalImageSamplerCount > 0)
		totalDescriptorTypeCount[vk::DescriptorType::eCombinedImageSampler] = totalImageSamplerCount;

	for(const Pipeline& pipeline: pipelines)
	{
		for(const auto& [type, count]: pipeline.getDescriptorTypeCount())
			totalDescriptorTypeCount[type] += count;
	}
	for(const FramebufferPipeline& fbPipeline: framebufferPipelines)
	{
		for(const auto& [type, count]: fbPipeline.getDescriptorTypeCount())
			totalDescriptorTypeCount[type] += count;
	}
	for(const RayTracingPipeline& rtPipeline: rayTracingPipelines)
	{
		for(const auto& [type, count]: rtPipeline.getDescriptorTypeCount())
			totalDescriptorTypeCount[type] += count;
		totalDescriptorTypeCount[vk::DescriptorType::eStorageImage]++;
	}

	std::vector<PoolSizeData> poolSizesInfo{totalDescriptorTypeCount.size()};
	uint32_t i = 0;
	for(const auto& [type, count]: totalDescriptorTypeCount)
	{
		poolSizesInfo[i].descriptorCount = count;
		poolSizesInfo[i].descriptorType = type;
		i++;
	}

	descriptorPool.createDescriptorPool(poolSizesInfo);

	for(uint32_t i = 0; i < pipelines.size(); i++)
	{
		const std::unique_ptr<MeshManager>& pMeshManager = meshManagers[i];
		if(pMeshManager->getMeshCount() == 0) continue;

		DescriptorSetHandler& descriptorSetHandler = pipelines[i].getDescriptorSetHandler(0);
		descriptorSetHandler.defineDescriptorSetsAmount(pMeshManager->getMaterialCount());
		descriptorPool.allocateDescriptorSet(descriptorSetHandler);

		pMeshManager->loadMeshes(descriptorSetHandler);
	}
	LOG_INFO("Meshes loaded to the GPU.\n");

	for(FramebufferPipeline& fbPipeline: framebufferPipelines)
	{
		DescriptorSetHandler& descriptorSetHandler = fbPipeline.getDescriptorSetHandler(0);
		descriptorSetHandler.defineDescriptorSetsAmount(1);
		descriptorPool.allocateDescriptorSet(descriptorSetHandler);
	}

	for(RayTracingPipeline& rtPipeline: rayTracingPipelines)
	{
		DescriptorSetHandler& descriptorSetHandler = rtPipeline.getDescriptorSetHandler(0);
		descriptorSetHandler.defineDescriptorSetsAmount(1);
		descriptorPool.allocateDescriptorSet(descriptorSetHandler);
	}
}

// Executes starting code on scene
void mtd::Scene::start() const
{
	for(const std::unique_ptr<MeshManager>& pMeshManager: meshManagers)
	{
		if(pMeshManager->getMeshCount() > 0)
			pMeshManager->start();
	}
}

// Updates scene data
void mtd::Scene::update(double frameTime) const
{
	for(const std::unique_ptr<MeshManager>& pMeshManager: meshManagers)
	{
		if(pMeshManager->getMeshCount() > 0)
			pMeshManager->update(frameTime);
	}
}

// Sums the texture count from all mesh managers
uint32_t mtd::Scene::getTotalTextureCount() const
{
	uint32_t count = 0;
	for(const std::unique_ptr<MeshManager>& pMeshManager: meshManagers)
		count += pMeshManager->getTextureCount();

	return count;
}

// Sums the amount of material float data descriptors
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
