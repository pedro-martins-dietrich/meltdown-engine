#pragma once

#include <memory>

#include "InstanceManager.hpp"
#include "ResourceManager.hpp"
#include "../AssetManager/TexturePool.hpp"
#include "../AssetManager/MaterialManager.hpp"
#include "../AssetManager/MeshPool.hpp"
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
			const MeshPool& getMeshPool() const { return meshPool; }
			const std::vector<SceneInstance>& getInstances() const { return instanceManager.getInstances(); }
			const DescriptorPool& getDescriptorPool() const { return descriptorPool; }
			const ResourceManager& getResourceManager() const { return resourceManager; }

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
			void configureSceneDescriptorSet(DescriptorSetHandler& descriptorSetHandler);

			// Executes starting code on scene
			void start() const;
			// Updates scene data
			void update(double frameTime) const;

		private:
			// Active mesh managers
			std::vector<std::unique_ptr<MeshManager>> meshManagers;

			// Scene textures
			TexturePool texturePool;
			// Scene materials
			MaterialManager materialManager;
			// Scene meshes
			MeshPool meshPool;
			// Scene instances
			InstanceManager instanceManager;
			// GPU resources
			ResourceManager resourceManager;

			// Descriptor pool for the pipelines descriptor sets
			DescriptorPool descriptorPool;

			// Sums the texture count from all mesh managers
			uint32_t getTotalTextureCount() const;
			// Sums the amount of material float data descriptors
			uint32_t getMaterialFloatDataCount() const;
	};
}
