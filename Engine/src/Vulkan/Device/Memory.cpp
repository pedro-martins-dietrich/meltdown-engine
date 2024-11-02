#include <pch.hpp>
#include "Memory.hpp"

#include "../../Utils/Logger.hpp"

// Creates and allocates a buffer
void mtd::Memory::createBuffer(const Device& device, Buffer& buffer)
{
	vk::BufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.flags = vk::BufferCreateFlags();
	bufferCreateInfo.size = buffer.size;
	bufferCreateInfo.usage = buffer.usage;
	bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;

	vk::Result result = device.getDevice().createBuffer(&bufferCreateInfo, nullptr, &(buffer.buffer));
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create buffer. Vulkan result: %d", result);
		return;
	}

	allocateBufferMemory(device, buffer);
}

// Allocates memory for the buffer
void mtd::Memory::allocateBufferMemory(const Device& device, Buffer& buffer)
{
	const vk::Device& vulkanDevice = device.getDevice();
	vk::MemoryRequirements memoryRequirements = vulkanDevice.getBufferMemoryRequirements(buffer.buffer);

	vk::MemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex
	(
		device.getPhysicalDevice(), memoryRequirements.memoryTypeBits, buffer.memoryProperties
	);

	vk::Result result = vulkanDevice.allocateMemory(&memoryAllocateInfo, nullptr, &(buffer.bufferMemory));
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to allocate memory for buffer. Vulkan result: %d", result);
		return;
	}

	vulkanDevice.bindBufferMemory(buffer.buffer, buffer.bufferMemory, 0);
}

// Copies data to buffer mapped memory
void mtd::Memory::copyMemory
(
	const vk::Device& device,
	const vk::DeviceMemory& bufferMemory,
	vk::DeviceSize size,
	const void* srcData
)
{
	void* memoryLocation = device.mapMemory(bufferMemory, 0, size);
	memcpy(memoryLocation, srcData, size);
	device.unmapMemory(bufferMemory);
}

// Copies one buffer to another
void mtd::Memory::copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, const CommandHandler& commandHandler)
{
	vk::DeviceSize copySize = (dstBuffer.size > srcBuffer.size) ? srcBuffer.size : dstBuffer.size;

	vk::CommandBuffer commandBuffer = commandHandler.beginSingleTimeCommand();

	vk::BufferCopy bufferCopy{};
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;
	bufferCopy.size = copySize;

	commandBuffer.copyBuffer(srcBuffer.buffer, dstBuffer.buffer, 1, &bufferCopy);

	commandHandler.endSingleTimeCommand(commandBuffer);
}

// Changes the buffer size by reallocating it
void mtd::Memory::resizeBuffer(const Device& device, Buffer& buffer, vk::DeviceSize newSize)
{
	Buffer newBuffer;
	newBuffer.size = newSize;
	newBuffer.usage = buffer.usage | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
	newBuffer.memoryProperties = buffer.memoryProperties;

	createBuffer(device, newBuffer);
	CommandHandler commandHandler{device};
	copyBuffer(buffer, newBuffer, commandHandler);

	device.getDevice().destroyBuffer(buffer.buffer);
	device.getDevice().freeMemory(buffer.bufferMemory);

	buffer.buffer = newBuffer.buffer;
	buffer.bufferMemory = newBuffer.bufferMemory;
	buffer.size = newSize;
}

// Finds the memory type index that fits the requirements
uint32_t mtd::Memory::findMemoryTypeIndex
(
	const vk::PhysicalDevice& physicalDevice,
	uint32_t supportedMemoryIndex,
	vk::MemoryPropertyFlags requestedProperties
)
{
	vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

	for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		bool supported = static_cast<bool>(supportedMemoryIndex & (1U << i));
		bool sufficient =
			(memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties;

		if(supported && sufficient) return i;
	}

	LOG_ERROR("Could not find memory type index with the requested properties.");
	return 0;
}
