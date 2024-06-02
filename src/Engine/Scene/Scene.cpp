#include "Scene.hpp"

#include "../Utils/Logger.hpp"

mtd::Scene::Scene(const Device& device)
{
	meshManagers.emplace(PipelineType::DEFAULT, std::make_unique<DefaultMeshManager>(device));
	meshManagers.emplace(PipelineType::BILLBOARD, std::make_unique<BillboardManager>(device));
}

// TODO: Remove this getter
uint32_t mtd::Scene::getInstanceCount() const
{
	return dynamic_cast<DefaultMeshManager*>
	(
		meshManagers.at(PipelineType::DEFAULT).get()
	)->getTotalInstanceCount();
}

// Loads scene from file
void mtd::Scene::loadScene(const char* sceneFileName, const CommandHandler& commandHandler)
{
	SceneLoader::load(sceneFileName, meshes);

	DefaultMeshManager* defaultMeshManager =
		dynamic_cast<DefaultMeshManager*>(meshManagers.at(PipelineType::DEFAULT).get());

	for(Mesh& mesh: meshes)
	{
		defaultMeshManager->loadMeshToLump(mesh);
	}
	defaultMeshManager->loadMeshesToGPU(commandHandler);

	LOG_INFO("Meshes loaded to the GPU.");
}

// Loads textures associated to meshes
void mtd::Scene::loadTextures
(
	const Device& mtdDevice,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& textureDescriptorSetHandler
)
{
	diffuseTextures.resize(meshes.size());
	for(uint32_t i = 0; i < meshes.size(); i++)
	{
		diffuseTextures[i] = std::make_unique<Texture>
		(
			mtdDevice,
			meshes[i].getTexturePath().c_str(),
			commandHandler,
			textureDescriptorSetHandler,
			i
		);
	}

	LOG_INFO("Scene textures loaded.\n");
}
