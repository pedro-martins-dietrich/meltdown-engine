#pragma once

#include "RayTracingBLAS.hpp"

namespace mtd
{
	// Top Level Acceleration Structure for the ray tracing pipeline
	class RayTracingTLAS
	{
		public:
			RayTracingTLAS(const Device& mtdDevice, ResourceManager& resourceManager);
			~RayTracingTLAS();

			RayTracingTLAS(RayTracingTLAS&& other) noexcept;

			// Creates and builds the acceleration structure
			void create
			(
				const std::vector<SceneInstance> sceneInstances,
				const std::vector<RayTracingBLAS>& blasList,
				const CommandHandler& commandHandler
			);
			// Clears TLAS data
			void clear();

		private:
			// Vulkan acceleration structure handle
			vk::AccelerationStructureKHR accelerationStructure;
			// GPU buffer for the bottom level acceleration structure
			ResourceID bufferID = 0U;

			// Device reference
			const Device& mtdDevice;
			// Reference to the engine's resource manager
			ResourceManager& resourceManager;

			// Creates an instance buffer from the BLAS list
			ResourceID createInstanceBuffer
			(
				const std::vector<SceneInstance> sceneInstances,
				const std::vector<RayTracingBLAS>& blasList,
				const CommandHandler& commandHandler
			) const;

			// Handles the creation and build logic for the TLAS
			void createAndBuild
			(
				const CommandHandler& commandHandler,
				const vk::AccelerationStructureGeometryKHR& geometry,
				uint32_t instanceCount
			);
	};
}
