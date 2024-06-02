#pragma once

#include <memory>

#include "SceneLoader.hpp"
#include "../Vulkan/Mesh/Managers/DefaultMeshManager.hpp"
#include "../Vulkan/Mesh/Managers/BillboardManager.hpp"
#include "../Vulkan/Image/Texture.hpp"

namespace mtd
{
	// Stores the meshes present in the scene
	class Scene
	{
		public:
			Scene(const Device& device);
			~Scene() = default;

			Scene(const Scene&) = delete;
			Scene& operator=(const Scene&) = delete;

			// Getters
			std::vector<Mesh>& getMeshes() { return meshes; }
			Mesh& getMesh(uint32_t index) { return meshes[index]; }
			const MeshManager* getMeshManager(PipelineType type) const
				{ return meshManagers.at(type).get(); }
			uint32_t getInstanceCount() const;

			// Loads scene from file
			void loadScene(const char* sceneFileName, const CommandHandler& commandHandler);

			// Loads textures associated to meshes
			void loadTextures
			(
				const Device& mtdDevice,
				const CommandHandler& commandHandler,
				DescriptorSetHandler& textureDescriptorSetHandler
			);

		private:
			// Active mesh managers
			std::unordered_map<PipelineType, std::unique_ptr<MeshManager>> meshManagers;

			// List of meshes used in the scene
			std::vector<Mesh> meshes;
			std::vector<std::unique_ptr<Texture>> diffuseTextures;
	};
}
