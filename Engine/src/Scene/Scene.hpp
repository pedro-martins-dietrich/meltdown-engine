#pragma once

#include <memory>

#include "InstanceManager.hpp"
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
			const std::vector<MeshData>& getMeshes() const { return meshes; }
			const std::vector<SceneInstance>& getInstances() const { return instanceManager.getInstances(); }
			const DescriptorPool& getDescriptorPool() const { return descriptorPool; }

			// Setter
			void setCameraResourceID(ResourceID cameraResourceID) { gpuResources.cameraResourceID = cameraResourceID; }

			// Loads scene from file
			void loadScene
			(
				const Device& mtdDevice,
				std::string_view sceneFileName,
				std::vector<FramebufferInfo>& framebufferInfos,
				PipelineInfoBundle& pipelineInfos,
				ResourceManager& resourceManager,
				DescriptorManager& descriptorManager,
				std::vector<RenderPassInfo>& renderOrder
			);

			// Allocates resources and loads all mesh data
			void allocateResources(PipelineBundle& pipelines);
			// Configures the scene descriptor set with the scene data
			void configureSceneDescriptorSet
			(
				const ResourceManager& resourceManager, DescriptorSetHandler& descriptorSetHandler
			);

			// Binds the vertex and index buffers
			void bindMeshData(const ResourceManager& resourceManager, vk::CommandBuffer commandBuffer) const;

			// Executes starting code on scene
			void start() const;
			// Updates scene data
			void update(double frameTime) const;

		private:
			// Active mesh managers
			std::vector<std::unique_ptr<MeshManager>> meshManagers;

			// GPU resources managed by the engine
			SceneResources gpuResources;
			// Scene instances
			InstanceManager instanceManager;

			// CPU side data for all scene meshes
			std::vector<MeshData> meshes;

			// Descriptor pool for the pipelines descriptor sets
			DescriptorPool descriptorPool;

			// Sums the texture count from all mesh managers
			uint32_t getTotalTextureCount() const;
			// Sums the amount of material float data descriptors
			uint32_t getMaterialFloatDataCount() const;
	};
}
