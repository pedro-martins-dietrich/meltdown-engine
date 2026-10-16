#pragma once

#include "InstanceManager.hpp"
#include "../Vulkan/AccelerationStructure/RayTracingBVH.hpp"
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

			// Binds the vertex and index buffers
			void bindMeshData(const ResourceManager& resourceManager, vk::CommandBuffer commandBuffer) const;

			// Executes starting code on scene
			void start() const;
			// Updates scene data
			void update(double frameTime) const;

		private:
			// GPU resources managed by the engine
			SceneResources gpuResources;
			// Scene instances
			InstanceManager instanceManager;

			// CPU side data for all scene meshes
			std::vector<MeshData> meshes;

			// BVH structure for ray tracing
			std::optional<RayTracingBVH> rayTracingBVH;

			// Descriptor pool for the pipelines descriptor sets
			DescriptorPool descriptorPool;
	};
}
