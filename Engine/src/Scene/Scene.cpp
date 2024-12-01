#include <pch.hpp>
#include "Scene.hpp"

#include "../Utils/Logger.hpp"
#include "../Vulkan/Mesh/DefaultMesh/DefaultMeshManager.hpp"
#include "../Vulkan/Mesh/Billboard/BillboardManager.hpp"

mtd::Scene::Scene(const Device& device) : descriptorPool{device.getDevice()}
{
}

// Loads scene from file
void mtd::Scene::loadScene(const Device& device, const char* sceneFileName, std::vector<PipelineInfo>& pipelineInfos)
{
	meshManagers.clear();

	SceneLoader::load(device, sceneFileName, pipelineInfos, meshManagers);
}

// Allocate resources and loads all mesh data
void mtd::Scene::loadMeshes(std::vector<Pipeline>& pipelines)
{
	descriptorPool.clear();

	std::unordered_map<vk::DescriptorType, uint32_t> totalDescriptorTypeCount;
	totalDescriptorTypeCount[vk::DescriptorType::eUniformBuffer] = 1;
	totalDescriptorTypeCount[vk::DescriptorType::eCombinedImageSampler] = getTotalTextureCount();

	for(const Pipeline& pipeline: pipelines)
	{
		for(const auto& [type, count]: pipeline.getDescriptorTypeCount())
			totalDescriptorTypeCount[type] += count;
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
		descriptorSetHandler.defineDescriptorSetsAmount(pMeshManager->getMeshCount());
		descriptorPool.allocateDescriptorSet(descriptorSetHandler);

		pMeshManager->loadMeshes(descriptorSetHandler);
	}

	LOG_INFO("Meshes loaded to the GPU.\n");
}

// Executes starting code on scene
void mtd::Scene::start() const
{
	for(const std::unique_ptr<MeshManager>& pMeshManager : meshManagers)
	{
		if(pMeshManager->getMeshCount() > 0)
			pMeshManager->start();
	}
}

// Updates scene data
void mtd::Scene::update(double frameTime) const
{
	for(const std::unique_ptr<MeshManager>& pMeshManager : meshManagers)
	{
		if(pMeshManager->getMeshCount() > 0)
			pMeshManager->update(frameTime);
	}
}

// Sums the texture count from all mesh managers
uint32_t mtd::Scene::getTotalTextureCount() const
{
	uint32_t count = 0;
	for(const std::unique_ptr<MeshManager>& pMeshManager : meshManagers)
		count += pMeshManager->getMeshCount();

	return count;
}
