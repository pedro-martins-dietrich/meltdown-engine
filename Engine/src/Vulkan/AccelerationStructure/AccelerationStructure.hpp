#pragma once

#include "../Device/GpuBuffer.hpp"

namespace mtd
{
	// Manages Vulkan KHR acceleration structures
	class AccelerationStructure
	{
		public:
			AccelerationStructure(const Device& device);
			~AccelerationStructure();

			AccelerationStructure(const AccelerationStructure&) = delete;
			AccelerationStructure& operator=(const AccelerationStructure&) = delete;

			// Getters
			const vk::AccelerationStructureKHR& getAccelerationStructure() const { return accelerationStructure; }
			const GpuBuffer& getBuffer() const { return asBuffer; }

			// Setter
			void setScratchBufferAddress(vk::DeviceAddress scratchBufferAddress)
				{ asBuildGeometryInfo.scratchData.deviceAddress = scratchBufferAddress; }

			// Creates the Bottom Level Acceleration Structure (BLAS)
			void createBLAS
			(
				const CommandHandler& commandHandler,
				vk::DeviceAddress vertexBufferAddress,
				uint32_t vertexCount,
				vk::DeviceAddress indexBufferAddress,
				uint32_t triangleCount
			);
			// Creates the Top Level Acceleration Structure (TLAS)
			void createTLAS(const CommandHandler& commandHandler, vk::DeviceAddress instanceBufferAddress);
			// Creates an instance buffer from BLAS
			GpuBuffer createInstanceBuffer() const;

		private:
			// Vulkan acceleration structure handle
			vk::AccelerationStructureKHR accelerationStructure;
			// GPU buffer for the acceleration structure
			GpuBuffer asBuffer;

			// Acceleration structure data
			vk::AccelerationStructureTypeKHR asType;
			vk::AccelerationStructureGeometryKHR geometry;
			vk::AccelerationStructureBuildGeometryInfoKHR asBuildGeometryInfo;

			// Device reference
			const Device& device;

			// Handles the common creation logic for both BLAS and TLAS
			void createAS(const CommandHandler& commandHandler, uint32_t primitiveCount);
			// Builds the acceleration structure in the GPU
			void buildAS(const CommandHandler& commandHandler, uint32_t primitiveCount) const;
	};
}
