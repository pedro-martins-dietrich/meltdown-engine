#pragma once

#include <memory>

#include "SceneLoader.hpp"
#include "../Vulkan/Mesh/DefaultMesh/DefaultMeshManager.hpp"
#include "../Vulkan/Mesh/Billboard/BillboardManager.hpp"
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
			std::vector<DefaultMesh>& getMeshes();
			DefaultMesh& getMesh(uint32_t index);
			uint32_t getInstanceCount() const;

			// Loads scene from file
			void loadScene
			(
				const char* sceneFileName,
				const CommandHandler& commandHandler,
				std::unordered_map<PipelineType, Pipeline>& pipelines
			);

			// Updates scene data
			void update() const;

		private:
			// Active mesh managers
			std::unordered_map<PipelineType, std::unique_ptr<MeshManager>> meshManagers;

			// Descriptor pool for the pipelines descriptor sets
			DescriptorPool descriptorPool;

			// Allocate resources and loads all mesh data
			void loadMeshes
			(
				const CommandHandler& commandHandler,
				std::unordered_map<PipelineType, Pipeline>& pipelines
			);
	};
}
