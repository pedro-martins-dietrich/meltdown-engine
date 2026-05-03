#pragma once

#include <memory>

#include "../AssetManager/TexturePool.hpp"
#include "../Vulkan/Mesh/MeshManager.hpp"
#include "../Vulkan/Descriptors/DescriptorPool.hpp"
#include "../Vulkan/Pipeline/PipelineBundles.hpp"

namespace mtd
{
	// Stores the meshes present in the scene
	class Scene
	{
		public:
			Scene(const Device& mtdDevice);
			~Scene() = default;

			Scene(const Scene&) = delete;
			Scene& operator=(const Scene&) = delete;

			// Getters
			const MeshManager* getMeshManager(uint32_t pipelineIndex) const
				{ return meshManagers[pipelineIndex].get(); }
			const DescriptorPool& getDescriptorPool() const { return descriptorPool; }

			// Loads scene from file
			void loadScene
			(
				const Device& mtdDevice,
				std::string_view sceneFileName,
				std::vector<FramebufferInfo>& framebufferInfos,
				PipelineInfoBundle& pipelineInfos,
				std::vector<RenderPassInfo>& renderOrder
			);

			// Allocates resources and loads all mesh data
			void allocateResources(PipelineBundle& pipelines);
			// Configures the scene descriptor set with the scene data
			void configureSceneDescriptorSet(DescriptorSetHandler& descriptorSetHandler) const;

			// Executes starting code on scene
			void start() const;
			// Updates scene data
			void update(double frameTime) const;

		private:
			// Active mesh managers
			std::vector<std::unique_ptr<MeshManager>> meshManagers;

			// Scene textures
			TexturePool texturePool;

			// Descriptor pool for the pipelines descriptor sets
			DescriptorPool descriptorPool;

			// Sums the texture count from all mesh managers
			uint32_t getTotalTextureCount() const;
			// Sums the amount of material float data descriptors
			uint32_t getMaterialFloatDataCount() const;
	};
}
