#include <pch.hpp>
#include "Scene.hpp"

#include "SceneLoader.hpp"
#include "../Utils/Logger.hpp"
#include "../Vulkan/Mesh/MeshManager.hpp"

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
	meshManagers.clear();

	SceneLoader::load
	(
		mtdDevice,
		sceneFileName,
		framebufferInfos,
		pipelineInfos,
		renderOrder,
		meshManagers,
		meshes,
		resourceManager,
		descriptorManager,
		instanceManager,
		gpuResources
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

void mtd::Scene::configureSceneDescriptorSet
(
	const ResourceManager& resourceManager, DescriptorSetHandler& descriptorSetHandler
)
{
	vk::DescriptorBufferInfo bufferInfo{};
	std::vector<vk::DescriptorImageInfo> descriptorImageInfos;

	gpuResources.materialBufferID = resourceManager.getResourceID("MaterialBuffer");
	gpuResources.materialIndexingBufferID = resourceManager.getResourceID("MaterialIndexingBuffer");
	gpuResources.materialSetBufferID = resourceManager.getResourceID("MaterialSetBuffer");

	if(resourceManager.fetchDescriptorBufferInfo(gpuResources.cameraResourceID, bufferInfo))
		descriptorSetHandler.assignBuffer(0U, bufferInfo);
	if(resourceManager.fetchDescriptorBufferInfo(gpuResources.vertexBufferID, bufferInfo))
		descriptorSetHandler.assignBuffer(1U, bufferInfo);
	if(resourceManager.fetchDescriptorBufferInfo(gpuResources.indexBufferID, bufferInfo))
		descriptorSetHandler.assignBuffer(2U, bufferInfo);
	if(resourceManager.fetchDescriptorBufferInfo(gpuResources.submeshBufferID, bufferInfo))
		descriptorSetHandler.assignBuffer(3U, bufferInfo);
	if(resourceManager.fetchDescriptorImageInfos(gpuResources.textureIDs, descriptorImageInfos))
		descriptorSetHandler.createImagesDescriptorResources(4U, descriptorImageInfos);
	if(resourceManager.fetchDescriptorBufferInfo(gpuResources.materialBufferID, bufferInfo))
		descriptorSetHandler.assignBuffer(5U, bufferInfo);
	if(resourceManager.fetchDescriptorBufferInfo(gpuResources.materialIndexingBufferID, bufferInfo))
		descriptorSetHandler.assignBuffer(6U, bufferInfo);
	if(resourceManager.fetchDescriptorBufferInfo(gpuResources.materialSetBufferID, bufferInfo))
		descriptorSetHandler.assignBuffer(7U, bufferInfo);
}

void mtd::Scene::bindMeshData(const ResourceManager& resourceManager, vk::CommandBuffer commandBuffer) const
{
	vk::DeviceSize offset{0UL};
    vk::Buffer vertexBuffer = resourceManager.getVulkanBuffer(gpuResources.vertexBufferID);
    vk::Buffer indexBuffer = resourceManager.getVulkanBuffer(gpuResources.indexBufferID);

    commandBuffer.bindVertexBuffers(0U, 1U, &vertexBuffer, &offset);
    commandBuffer.bindIndexBuffer(indexBuffer, offset, vk::IndexType::eUint32);
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

	count += gpuResources.textureIDs.size() + 1U;

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
