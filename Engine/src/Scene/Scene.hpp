#pragma once

#include <memory>

#include "SceneLoader.hpp"
#include "../Vulkan/Mesh/MeshManager.hpp"
#include "../Vulkan/Pipeline/Pipeline.hpp"
#include "../Vulkan/Descriptors/DescriptorPool.hpp"

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
			const MeshManager* getMeshManager(uint32_t pipelineIndex) const
				{ return meshManagers[pipelineIndex].get(); }
			const DescriptorPool& getDescriptorPool() const { return descriptorPool; }

			// Loads scene from file
			void loadScene(const Device& device, const char* sceneFileName, std::vector<PipelineInfo>& pipelineInfos);

			// Allocate resources and loads all mesh data
			void loadMeshes(std::vector<Pipeline>& pipelines);

			// Executes starting code on scene
			void start() const;
			// Updates scene data
			void update(double frameTime) const;

		private:
			// Active mesh managers
			std::vector<std::unique_ptr<MeshManager>> meshManagers;

			// Descriptor pool for the pipelines descriptor sets
			DescriptorPool descriptorPool;

			// Sums the texture count from all mesh managers
			uint32_t getTotalTextureCount() const;
	};
}
