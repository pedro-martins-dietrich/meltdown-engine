#include "Scene.hpp"

#include "../Utils/Logger.hpp"

mtd::Scene::Scene(const Device& device) : descriptorPool{device.getDevice()}
{
	meshManagers.emplace(PipelineType::DEFAULT, std::make_unique<DefaultMeshManager>(device));
	meshManagers.emplace(PipelineType::BILLBOARD, std::make_unique<BillboardManager>(device));
}

// TODO: Remove these getters
uint32_t mtd::Scene::getInstanceCount() const
{
	return dynamic_cast<DefaultMeshManager*>
	(
		meshManagers.at(PipelineType::DEFAULT).get()
	)->getTotalInstanceCount();
}
std::vector<mtd::Mesh>& mtd::Scene::getMeshes()
{
	return dynamic_cast<DefaultMeshManager*>
	(
		meshManagers.at(PipelineType::DEFAULT).get()
	)->getMeshes();
}
mtd::Mesh& mtd::Scene::getMesh(uint32_t index)
{
	return dynamic_cast<DefaultMeshManager*>
	(
		meshManagers.at(PipelineType::DEFAULT).get()
	)->getMeshes()[index];
}

// Loads scene from file
void mtd::Scene::loadScene
(
	const char* sceneFileName,
	const CommandHandler& commandHandler,
	std::unordered_map<PipelineType, Pipeline>& pipelines
)
{
	DefaultMeshManager* defaultMeshManager =
		dynamic_cast<DefaultMeshManager*>(meshManagers.at(PipelineType::DEFAULT).get());

	SceneLoader::load(sceneFileName, defaultMeshManager->getMeshes());
	defaultMeshManager->loadMeshes(commandHandler);
	LOG_INFO("Meshes loaded to the GPU.\n");

	loadTextures(commandHandler, pipelines);
}

// Loads textures associated to meshes
void mtd::Scene::loadTextures
(
	const CommandHandler& commandHandler,
	std::unordered_map<PipelineType, Pipeline>& pipelines
)
{
	std::vector<PoolSizeData> poolSizesInfo{3};
	poolSizesInfo[0].descriptorCount = 1;
	poolSizesInfo[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	poolSizesInfo[1].descriptorCount = 1;
	poolSizesInfo[1].descriptorType = vk::DescriptorType::eUniformBuffer;
	poolSizesInfo[2].descriptorCount = static_cast<uint32_t>(getMeshes().size() + 1);
	poolSizesInfo[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descriptorPool.createDescriptorPool(poolSizesInfo);

	for(auto& [type, pMeshManager]: meshManagers)
	{
		DescriptorSetHandler& descriptorSetHandler = pipelines.at(type).getDescriptorSetHandler(0);
		descriptorSetHandler.defineDescriptorSetsAmount(pMeshManager->getMeshCount());
		descriptorPool.allocateDescriptorSet(descriptorSetHandler);

		pMeshManager->loadTextures(commandHandler, descriptorSetHandler);
	}

	LOG_INFO("All textures loaded.\n");
}
