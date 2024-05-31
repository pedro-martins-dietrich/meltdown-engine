#include "Scene.hpp"

#include "../Utils/Logger.hpp"

mtd::Scene::Scene(const Device& device) : defaultMeshManager{device}
{
}

// Loads scene from file
void mtd::Scene::loadScene(const char* sceneFileName, const CommandHandler& commandHandler)
{
	SceneLoader::load(sceneFileName, meshes);

	for(Mesh& mesh: meshes)
	{
		defaultMeshManager.loadMeshToLump(mesh);
	}
	defaultMeshManager.loadMeshesToGPU(commandHandler);

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
