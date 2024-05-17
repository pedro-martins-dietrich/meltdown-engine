#pragma once

#include <memory>

#include "SceneLoader.hpp"
#include "../Vulkan/Mesh/MeshManager.hpp"
#include "../Vulkan/Image/Texture.hpp"

namespace mtd
{
	// Stores the meshes present in the scene
	class Scene
	{
		public:
			Scene() {}
			~Scene() {}

			Scene(const Scene&) = delete;
			Scene& operator=(const Scene&) = delete;

			// Getters
			std::vector<Mesh>& getMeshes() { return meshes; }
			Mesh& getMesh(uint32_t index) { return meshes[index]; }

			// Loads scene from file
			void loadScene
			(
				const char* sceneFileName,
				MeshManager& meshManager,
				const CommandHandler& commandHandler
			);

			// Loads textures associated to meshes
			void loadTextures
			(
				const Device& mtdDevice,
				const CommandHandler& commandHandler,
				DescriptorSetHandler& textureDescriptorSetHandler
			);

		private:
			// List of meshes used in the scene
			std::vector<Mesh> meshes;
			std::vector<std::unique_ptr<Texture>> diffuseTextures;
	};
}
