#include <pch.hpp>
#include "ScratchAccelerationStructure.hpp"

#include "../../Utils/Logger.hpp"

mtd::ScratchAccelerationStructure::ScratchAccelerationStructure
(
	const Device& device, vk::DeviceSize buildScratchSize, vk::AccelerationStructureTypeKHR asType
) : device{device},
	accelerationStructure{nullptr},
	asBuffer
	{
		device,
		buildScratchSize,
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress
		| vk::BufferUsageFlagBits::eStorageBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	}
{
	vk::AccelerationStructureCreateInfoKHR asScratchCreateInfo{};
	asScratchCreateInfo.createFlags = vk::AccelerationStructureCreateFlagsKHR();
	asScratchCreateInfo.buffer = asBuffer.getBuffer();
	asScratchCreateInfo.offset = 0;
	asScratchCreateInfo.size = buildScratchSize;
	asScratchCreateInfo.type = asType;
	asScratchCreateInfo.deviceAddress = 0;

	vk::Result result = device.getDevice()
		.createAccelerationStructureKHR(&asScratchCreateInfo, nullptr, &accelerationStructure, device.getDLDI());
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create scratch acceleration structure. Vulkan result: %d", result);
}

mtd::ScratchAccelerationStructure::~ScratchAccelerationStructure()
{
	device.getDevice().destroyAccelerationStructureKHR(accelerationStructure, nullptr, device.getDLDI());
}
