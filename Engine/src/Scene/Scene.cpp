#include <pch.hpp>
#include "Scene.hpp"

#include "SceneLoader.hpp"
#include "../Utils/Logger.hpp"
#include "../Vulkan/Mesh/MeshManager.hpp"

mtd::Scene::Scene(const Device& mtdDevice)
	: texturePool{mtdDevice}, materialManager{mtdDevice}, meshPool{mtdDevice},
	instanceManager{}, resourceManager{mtdDevice},
	descriptorPool{mtdDevice.getDevice()}
{}

void mtd::Scene::loadScene
(
	const Device& mtdDevice,
	std::string_view sceneFileName,
	std::vector<FramebufferInfo>& framebufferInfos,
	PipelineInfoBundle& pipelineInfos,
	std::vector<RenderPassInfo>& renderOrder
)
{
	renderOrder.clear();
	meshManagers.clear();
	resourceManager.clearResources();

	SceneLoader::load
	(
		mtdDevice,
		sceneFileName,
		framebufferInfos,
		pipelineInfos,
		renderOrder,
		meshManagers,
		instanceManager,
		texturePool,
		materialManager,
		meshPool
	);
}

void mtd::Scene::allocateResources(PipelineBundle& pipelines)
{
	descriptorPool.clear();

	std::unordered_map<vk::DescriptorType, uint32_t> totalDescriptorTypeCount;
	totalDescriptorTypeCount[vk::DescriptorType::eUniformBuffer] = 1U + getMaterialFloatDataCount();
	totalDescriptorTypeCount[vk::DescriptorType::eStorageBuffer] = 7U;

	uint32_t totalImageSamplerCount = getTotalTextureCount();
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
	{
		for(const auto& [type, count]: rtPipeline.getDescriptorTypeCount())
			totalDescriptorTypeCount[type] += count;
		totalDescriptorTypeCount[vk::DescriptorType::eAccelerationStructureKHR]++;
		totalDescriptorTypeCount[vk::DescriptorType::eStorageImage] += 2U;
		totalDescriptorTypeCount[vk::DescriptorType::eStorageBuffer] += 4U;
	}

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

		vk::DescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
		variableCountInfo.descriptorSetCount = 1U;
		variableCountInfo.pDescriptorCounts = &totalImageSamplerCount;
		descriptorPool.allocateDescriptorSet(descriptorSetHandler, &variableCountInfo);

		meshManagers[i]->loadMeshes(descriptorSetHandler);
	}
	LOG_INFO("Meshes loaded to the GPU.\n");
}

void mtd::Scene::configureSceneDescriptorSet(DescriptorSetHandler& descriptorSetHandler)
{
	meshPool.createMeshDescriptors(descriptorSetHandler, 1U, 2U, 3U);
	texturePool.createTextureListDescriptor(descriptorSetHandler, 4U);
	materialManager.createMaterialDescriptor(descriptorSetHandler, 5U, 6U, 7U);
}

void mtd::Scene::start() const
{
	for(const std::unique_ptr<MeshManager>& pMeshManager: meshManagers)
	{
		if(pMeshManager->getMeshCount() > 0U)
			pMeshManager->start();
	}
}

void mtd::Scene::update(double frameTime) const
{
	for(const std::unique_ptr<MeshManager>& pMeshManager: meshManagers)
	{
		if(pMeshManager->getMeshCount() > 0U)
			pMeshManager->update(frameTime);
	}
}

uint32_t mtd::Scene::getTotalTextureCount() const
{
	uint32_t count = 0U;
	for(const std::unique_ptr<MeshManager>& pMeshManager: meshManagers)
		count += pMeshManager->getTextureCount();

	count += MAX_TEXTURE_POOL_SIZE;

	return count;
}

uint32_t mtd::Scene::getMaterialFloatDataCount() const
{
	uint32_t count = 0U;
	for(const std::unique_ptr<MeshManager>& pMeshManager: meshManagers)
	{
		if(pMeshManager->hasMaterialFloatData())
			count += pMeshManager->getMaterialCount();
	}
	return count;
}
