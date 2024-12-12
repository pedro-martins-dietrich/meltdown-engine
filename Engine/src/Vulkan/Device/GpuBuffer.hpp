#pragma once

#include "../Command/CommandHandler.hpp"

namespace mtd
{
	// Buffer for allocation in the GPU memory
	class GpuBuffer
	{
		public:
			GpuBuffer(const Device& device, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memoryProperties);
			GpuBuffer
			(
				const Device& device,
				vk::DeviceSize size,
				vk::BufferUsageFlags usage,
				vk::MemoryPropertyFlags memoryProperties
			);
			~GpuBuffer();

			GpuBuffer(const GpuBuffer&) = delete;
			GpuBuffer& operator=(const GpuBuffer&) = delete;

			GpuBuffer(GpuBuffer&& other) noexcept;

			// Getters
			const vk::Buffer& getBuffer() const { return buffer; }
			const vk::DeviceMemory& getBufferMemory() const { return bufferMemory; }
			vk::DeviceSize getSize() const { return size; }

			// Initializes the buffer
			void create(vk::DeviceSize dataSize);
			// Initializes the buffer using device local memory
			void createDeviceLocal(const CommandHandler& commandHandler, vk::DeviceSize dataSize, const void* data);

			// Changes the buffer size by reallocating it
			void resizeBuffer(const CommandHandler& commandHandler, vk::DeviceSize newSize);

			// Copies data to the buffer
			void copyMemoryToBuffer(vk::DeviceSize copySize, const void* srcData);

		private:
			// Vulkan buffer handles
			vk::Buffer buffer;
			vk::DeviceMemory bufferMemory;

			// Buffer data
			vk::DeviceSize size;
			vk::BufferUsageFlags usage;
			vk::MemoryPropertyFlags memoryProperties;

			// Device reference
			const Device& device;

			// Allocates memory for the buffer
			void allocateBufferMemory();
			// Copies the data from another buffer to this buffer
			void copyDataFromBuffer(const GpuBuffer& srcBuffer, const CommandHandler& commandHandler) const;
	};

	// Handles GPU memory management
	namespace Memory
	{
		// Finds the memory type index that fits the requirements
		uint32_t findMemoryTypeIndex
		(
			const vk::PhysicalDevice& physicalDevice,
			uint32_t supportedMemoryIndex,
			vk::MemoryPropertyFlags requestedProperties
		);
	}
}
