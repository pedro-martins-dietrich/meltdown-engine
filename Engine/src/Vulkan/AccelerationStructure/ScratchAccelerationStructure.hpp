#pragma once

#include "../Device/GpuBuffer.hpp"

namespace mtd
{
	// Handles scratch acceleration structures
	class ScratchAccelerationStructure
	{
		public:
			ScratchAccelerationStructure
			(
				const Device& device, vk::DeviceSize buildScratchSize, vk::AccelerationStructureTypeKHR asType
			);
			~ScratchAccelerationStructure();

			ScratchAccelerationStructure(const ScratchAccelerationStructure&) = delete;
			ScratchAccelerationStructure& operator=(const ScratchAccelerationStructure&) = delete;

			// Getter
			vk::DeviceAddress getBufferAddress() const { return asBuffer.getBufferAddress(); }

		private:
			// Vulkan acceleration structure handle
			vk::AccelerationStructureKHR accelerationStructure;
			// GPU buffer for the acceleration structure
			GpuBuffer asBuffer;

			// Device reference
			const Device& device;
	};
}
