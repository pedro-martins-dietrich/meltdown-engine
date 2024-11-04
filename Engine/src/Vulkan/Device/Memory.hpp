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
		vk::DeviceSize size;
		vk::BufferUsageFlags usage;
		vk::MemoryPropertyFlags memoryProperties;
	};

	// Creates and allocates a buffer
	void createBuffer(const Device& device, Buffer& buffer);

	// Allocates memory for the buffer
	void allocateBufferMemory(const Device& device, Buffer& buffer);

	// Copies data to buffer mapped memory
	void copyMemory
	(
		const vk::Device& device, const vk::DeviceMemory& bufferMemory, vk::DeviceSize size, const void* srcData
	);

	// Copies one buffer to another
	void copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, const CommandHandler& commandHandler);

	// Changes the buffer size by reallocating it
	void resizeBuffer
	(
		const Device& device, const CommandHandler& commandHandler, Buffer& buffer, vk::DeviceSize newSize
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
		const Device& device, Buffer& buffer, const std::vector<T>& data, const CommandHandler& commandHandler
	)
	{
		Buffer stagingBuffer;
		stagingBuffer.size = data.size() * sizeof(T);
		stagingBuffer.usage = vk::BufferUsageFlagBits::eTransferSrc;
		stagingBuffer.memoryProperties =
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

		buffer.size = stagingBuffer.size;
		buffer.usage |= vk::BufferUsageFlagBits::eTransferDst;
		buffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

		createBuffer(device, stagingBuffer);
		copyMemory(device.getDevice(), stagingBuffer.bufferMemory, stagingBuffer.size, data.data());

		createBuffer(device, buffer);
		copyBuffer(stagingBuffer, buffer, commandHandler);

		device.getDevice().destroyBuffer(stagingBuffer.buffer);
		device.getDevice().freeMemory(stagingBuffer.bufferMemory);
	}
}
