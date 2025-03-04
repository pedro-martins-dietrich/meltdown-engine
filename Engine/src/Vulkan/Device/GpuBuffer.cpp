#include <pch.hpp>
#include "GpuBuffer.hpp"

#include "../../Utils/Logger.hpp"

mtd::GpuBuffer::GpuBuffer(const Device& device, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memoryProperties)
	: device{device}, size{0}, usage{usage}, memoryProperties{memoryProperties}, buffer{nullptr}, bufferMemory{nullptr}
{
}

mtd::GpuBuffer::GpuBuffer
(
	const Device& device,
	vk::DeviceSize size,
	vk::BufferUsageFlags usage,
	vk::MemoryPropertyFlags memoryProperties
) : device{device}, size{size}, usage{usage}, memoryProperties{memoryProperties}
{
	vk::BufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.flags = vk::BufferCreateFlags();
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;

	vk::Result result = device.getDevice().createBuffer(&bufferCreateInfo, nullptr, &buffer);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create buffer. Vulkan result: %d", result);
		return;
	}

	allocateBufferMemory();
}

mtd::GpuBuffer::~GpuBuffer()
{
	if(buffer && bufferMemory)
	{
		device.getDevice().destroyBuffer(buffer);
		device.getDevice().freeMemory(bufferMemory);
	}
}

mtd::GpuBuffer::GpuBuffer(GpuBuffer&& other) noexcept
	: device{other.device},
	size{other.size},
	usage{other.usage},
	memoryProperties{other.memoryProperties},
	buffer{other.buffer},
	bufferMemory{other.bufferMemory}
{
	other.buffer = nullptr;
	other.bufferMemory = nullptr;
}

// Initializes the buffer
void mtd::GpuBuffer::create(vk::DeviceSize dataSize)
{
	size = dataSize;

	vk::BufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.flags = vk::BufferCreateFlags();
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;

	vk::Result result = device.getDevice().createBuffer(&bufferCreateInfo, nullptr, &buffer);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create buffer. Vulkan result: %d", result);
		return;
	}

	allocateBufferMemory();
}

// Initializes the buffer using device local memory
void mtd::GpuBuffer::createDeviceLocal
(
	const CommandHandler& commandHandler, vk::DeviceSize dataSize, const void* data
)
{
	GpuBuffer stagingBuffer
	{
		device,
		dataSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	size = stagingBuffer.size;
	usage |= vk::BufferUsageFlagBits::eTransferDst;
	memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

	stagingBuffer.copyMemoryToBuffer(dataSize, data);

	create(stagingBuffer.size);
	copyDataFromBuffer(stagingBuffer, commandHandler);
}

// Changes the buffer size by reallocating it
void mtd::GpuBuffer::resizeBuffer(const CommandHandler& commandHandler, vk::DeviceSize newSize)
{
	GpuBuffer newBuffer
	{
		device,
		newSize,
		usage | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
		memoryProperties
	};
	copyDataFromBuffer(newBuffer, commandHandler);

	buffer = newBuffer.buffer;
	bufferMemory = newBuffer.bufferMemory;
	size = newSize;
	usage = newBuffer.usage;
}

// Copies data to the buffer
void mtd::GpuBuffer::copyMemoryToBuffer(vk::DeviceSize copySize, const void* srcData)
{
	if(copySize > size)
		copySize = size;

	void* memoryLocation = device.getDevice().mapMemory(bufferMemory, 0, copySize);
	memcpy(memoryLocation, srcData, copySize);
	device.getDevice().unmapMemory(bufferMemory);
}

// Allocates memory for the buffer
void mtd::GpuBuffer::allocateBufferMemory()
{
	const vk::Device& vulkanDevice = device.getDevice();
	vk::MemoryRequirements memoryRequirements = vulkanDevice.getBufferMemoryRequirements(buffer);

	vk::MemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
	if(usage & vk::BufferUsageFlagBits::eShaderDeviceAddress)
		memoryAllocateFlagsInfo.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;

	vk::MemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex =
		Memory::findMemoryTypeIndex(device.getPhysicalDevice(), memoryRequirements.memoryTypeBits, memoryProperties);
	memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;

	vk::Result result = vulkanDevice.allocateMemory(&memoryAllocateInfo, nullptr, &bufferMemory);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to allocate memory for buffer. Vulkan result: %d", result);
		return;
	}

	vulkanDevice.bindBufferMemory(buffer, bufferMemory, 0);
}

// Copies the data from another buffer to this buffer
void mtd::GpuBuffer::copyDataFromBuffer(const GpuBuffer& srcBuffer, const CommandHandler& commandHandler) const
{
	vk::DeviceSize copySize = (size > srcBuffer.size) ? srcBuffer.size : size;

	vk::CommandBuffer commandBuffer = commandHandler.beginSingleTimeCommand();

	vk::BufferCopy bufferCopy{};
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;
	bufferCopy.size = copySize;

	commandBuffer.copyBuffer(srcBuffer.buffer, buffer, 1, &bufferCopy);

	commandHandler.endSingleTimeCommand(commandBuffer);
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
		bool sufficient = (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties;

		if(supported && sufficient) return i;
	}

	LOG_ERROR("Could not find memory type index with the requested properties.");
	return 0;
}
