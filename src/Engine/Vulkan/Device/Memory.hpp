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
}
