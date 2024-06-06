#pragma once

#include "../Command/CommandHandler.hpp"

// Handles GPU memory allocation
namespace mtd::Memory
{
	// Buffer for allocation in the GPU memory
	struct Buffer
	{
		vk::Buffer buffer;
		vk::DeviceMemory bufferMemory;
	};

	// Creates and allocates a buffer
	void createBuffer
	(
		const Device& device,
		Buffer& buffer,
		vk::DeviceSize bufferSize,
		vk::BufferUsageFlags bufferUsage,
		vk::MemoryPropertyFlags memoryProperties
	);

	// Allocates memory for the buffer
	void allocateBufferMemory
	(
		const Device& device,
		Buffer& buffer,
		vk::MemoryPropertyFlags memoryProperties
	);

	// Copies data to buffer mapped memory
	void copyMemory
	(
		const vk::Device& device,
		const vk::DeviceMemory& bufferMemory,
		vk::DeviceSize size,
		const void* srcData
	);

	// Copies one buffer to another
	void copyBuffer
	(
		Buffer& srcBuffer,
		Buffer& dstBuffer,
		vk::DeviceSize size,
		const CommandHandler& commandHandler
	);

	// Finds the memory type index that fits the requirements
	uint32_t findMemoryTypeIndex
	(
		const vk::PhysicalDevice& physicalDevice,
		uint32_t supportedMemoryIndex,
		vk::MemoryPropertyFlags requestedProperties
	);

	// Creates buffer copies data to a device local memory
	template<typename T>
	void createDeviceLocalBuffer
	(
		const Device& device,
		Buffer& buffer,
		const std::vector<T>& data,
		vk::BufferUsageFlags bufferUsage,
		const CommandHandler& commandHandler
	)
	{
		Buffer stagingBuffer;

		vk::DeviceSize dataSize = data.size() * sizeof(T);
		createBuffer
		(
			device,
			stagingBuffer,
			dataSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		copyMemory(device.getDevice(), stagingBuffer.bufferMemory, dataSize, data.data());

		createBuffer
		(
			device,
			buffer,
			dataSize,
			vk::BufferUsageFlagBits::eTransferDst | bufferUsage,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);
		copyBuffer(stagingBuffer, buffer, dataSize, commandHandler);

		device.getDevice().destroyBuffer(stagingBuffer.buffer);
		device.getDevice().freeMemory(stagingBuffer.bufferMemory);
	}
}
