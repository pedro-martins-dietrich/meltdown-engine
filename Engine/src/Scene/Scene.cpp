#include "Scene.hpp"

#include "../Utils/Logger.hpp"

mtd::Scene::Scene(const Device& device) : descriptorPool{device.getDevice()}
{
	meshManagers.emplace(PipelineType::DEFAULT, std::make_unique<DefaultMeshManager>(device));
	meshManagers.emplace(PipelineType::BILLBOARD, std::make_unique<BillboardManager>(device));
}

// Loads scene from file
void mtd::Scene::loadScene
(
	const char* sceneFileName,
	const CommandHandler& commandHandler,
	std::unordered_map<PipelineType, Pipeline>& pipelines
)
{
	for(auto& [type, pMeshManager]: meshManagers)
		pMeshManager->clearMeshes();

	SceneLoader::load(sceneFileName, meshManagers);
	loadMeshes(commandHandler, pipelines);
}

// Updates scene data
void mtd::Scene::update(double frameTime) const
{
	for(auto& [type, pMeshManager]: meshManagers)
	{
		if(pMeshManager->getMeshCount() > 0)
			pMeshManager->update(frameTime);
	}
}

// Sums the texture count from all mesh managers
uint32_t mtd::Scene::getTotalTextureCount() const
{
	uint32_t count = 0;
	for(auto& [type, pMeshManager]: meshManagers)
		count += pMeshManager->getMeshCount();

	return count;
}

// Allocate resources and loads all mesh data
void mtd::Scene::loadMeshes
(
	const CommandHandler& commandHandler,
	std::unordered_map<PipelineType, Pipeline>& pipelines
)
{
	descriptorPool.clear();

	std::vector<PoolSizeData> poolSizesInfo{3};
	poolSizesInfo[0].descriptorCount = 1;
	poolSizesInfo[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	poolSizesInfo[1].descriptorCount = 1;
	poolSizesInfo[1].descriptorType = vk::DescriptorType::eUniformBuffer;
	poolSizesInfo[2].descriptorCount = getTotalTextureCount();
	poolSizesInfo[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descriptorPool.createDescriptorPool(poolSizesInfo);

	for(auto& [type, pMeshManager]: meshManagers)
	{
		if(pMeshManager->getMeshCount() > 0)
		{
			DescriptorSetHandler& descriptorSetHandler = pipelines.at(type).getDescriptorSetHandler(0);
			descriptorSetHandler.defineDescriptorSetsAmount(pMeshManager->getMeshCount());
			descriptorPool.allocateDescriptorSet(descriptorSetHandler);

			pMeshManager->loadMeshes(commandHandler, descriptorSetHandler);
		}
	}

	LOG_INFO("Meshes loaded to the GPU.\n");
}
