#pragma once

#include <memory>

#include "SceneLoader.hpp"
#include "../Vulkan/Mesh/Managers/DefaultMeshManager.hpp"
#include "../Vulkan/Mesh/Managers/BillboardManager.hpp"
#include "../Vulkan/Pipeline/Pipeline.hpp"

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
			const MeshManager* getMeshManager(PipelineType type) const
				{ return meshManagers.at(type).get(); }
			const DescriptorPool& getDescriptorPool() const { return descriptorPool; }
			std::vector<Mesh>& getMeshes();
			Mesh& getMesh(uint32_t index);
			uint32_t getInstanceCount() const;

			// Loads scene from file
			void loadScene
			(
				const char* sceneFileName,
				const CommandHandler& commandHandler,
				std::unordered_map<PipelineType, Pipeline>& pipelines
			);

		private:
			// Active mesh managers
			std::unordered_map<PipelineType, std::unique_ptr<MeshManager>> meshManagers;

			// Descriptor pool for the pipelines descriptor sets
			DescriptorPool descriptorPool;

			// Loads textures associated to meshes
			void loadTextures
			(
				const CommandHandler& commandHandler,
				std::unordered_map<PipelineType, Pipeline>& pipelines
			);
	};
}
