#pragma once

#include "../../AssetManager/ResourceManager.hpp"

namespace mtd
{
	// Bottom Level Acceleration Structure for the ray tracing pipeline
	class RayTracingBlas
	{
		public:
			RayTracingBlas
			(
				const Device& mtdDevice, ResourceManager& resourceManager,
				const CommandHandler& commandHandler, const MeshData& meshData
			);
			~RayTracingBlas();

			RayTracingBlas(const RayTracingBlas&) = delete;
			RayTracingBlas& operator=(const RayTracingBlas&) = delete;

			RayTracingBlas(RayTracingBlas&& other) noexcept;

			// Getter
			vk::DeviceAddress getBufferAddress() const { return resourceManager.getBufferAddress(bufferID); }

		private:
			// Vulkan acceleration structure handle
			vk::AccelerationStructureKHR accelerationStructure;
			// GPU buffer for the bottom-level acceleration structure
			ResourceID bufferID = 0U;

			// Device reference
			const Device& mtdDevice;
			// Reference to the engine's resource manager
			ResourceManager& resourceManager;

			// Handles the creation and build logic for the BLAS
			void createAndBuild
			(
				const CommandHandler& commandHandler,
				const std::vector<vk::AccelerationStructureGeometryKHR>& geometries,
				const std::vector<uint32_t>& primitiveCounts,
				const std::vector<uint32_t>& primitiveOffsets
			);
	};
}
