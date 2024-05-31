#pragma once

#include <memory>

#include "SceneLoader.hpp"
#include "../Vulkan/Mesh/Managers/DefaultMeshManager.hpp"
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
			const DefaultMeshManager& getMeshManager() const { return defaultMeshManager; }
			uint32_t getInstanceCount() const { return defaultMeshManager.getTotalInstanceCount(); }

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
			// Manager for the default meshes
			DefaultMeshManager defaultMeshManager;

			// List of meshes used in the scene
			std::vector<Mesh> meshes;
			std::vector<std::unique_ptr<Texture>> diffuseTextures;
	};
}
