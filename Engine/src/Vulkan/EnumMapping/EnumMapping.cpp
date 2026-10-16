#include <pch.hpp>
#include "EnumMapping.hpp"

#include "../../Utils/Logger.hpp"

vk::BufferUsageFlags mtd::EnumMapping::getBufferUsage(GpuBufferType bufferType)
{
	vk::BufferUsageFlags bufferUsage{};

	if((bufferType & GpuBufferType::Uniform) != GpuBufferType::None)
		bufferUsage |= vk::BufferUsageFlagBits::eUniformBuffer;
	if((bufferType & GpuBufferType::Storage) != GpuBufferType::None)
		bufferUsage |= vk::BufferUsageFlagBits::eStorageBuffer;
	if((bufferType & GpuBufferType::Vertex) != GpuBufferType::None)
		bufferUsage |= (vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer);
	if((bufferType & GpuBufferType::Index) != GpuBufferType::None)
		bufferUsage |= (vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eStorageBuffer);
	if((bufferType |= GpuBufferType::AccelerationStructure) != GpuBufferType::None)
		bufferUsage |=
		(
			vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
			| vk::BufferUsageFlagBits::eShaderDeviceAddress
		);
	if((bufferType & GpuBufferType::AccelerationStructureInput) != GpuBufferType::None)
		bufferUsage |=
		(
			vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
			| vk::BufferUsageFlagBits::eShaderDeviceAddress
		);

	if((bufferType & GpuBufferType::TransferSource) != GpuBufferType::None)
		bufferUsage |= vk::BufferUsageFlagBits::eTransferSrc;
	if((bufferType & GpuBufferType::TransferDestination) != GpuBufferType::None)
		bufferUsage |= vk::BufferUsageFlagBits::eTransferDst;
	if((bufferType & GpuBufferType::DeviceAddress) != GpuBufferType::None)
		bufferUsage |= vk::BufferUsageFlagBits::eShaderDeviceAddress;

	return bufferUsage;
}

vk::MemoryPropertyFlags mtd::EnumMapping::getMemoryProperties(GpuMemoryUsage memoryUsage)
{
	switch(memoryUsage)
	{
		case GpuMemoryUsage::CpuUpload:
		case GpuMemoryUsage::CpuUploadOnce:
			return vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		case GpuMemoryUsage::CpuReadback:
			return vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached;
		case GpuMemoryUsage::GpuOnly:
			return vk::MemoryPropertyFlagBits::eDeviceLocal;
		default:
			LOG_ERROR("Unhandled memory usage flag: %d", static_cast<uint32_t>(memoryUsage));
	}
	return vk::MemoryPropertyFlagBits::eDeviceLocal;
}

mtd::GpuMemoryUsage mtd::EnumMapping::defaultMemoryUsage(GpuBufferType bufferType)
{
	if
	(
		(bufferType & GpuBufferType::Uniform) != GpuBufferType::None
		|| (bufferType & GpuBufferType::AccelerationStructureInput) != GpuBufferType::None // Do I really need to compare with None?
	) return GpuMemoryUsage::CpuUpload;

	return GpuMemoryUsage::GpuOnly;
}

vk::DescriptorType mtd::EnumMapping::getDescriptorType(DescriptorType descriptorType)
{
	switch(descriptorType)
	{
		case DescriptorType::UniformBuffer:
			return vk::DescriptorType::eUniformBuffer;
		case DescriptorType::StorageBuffer:
			return vk::DescriptorType::eStorageBuffer;
		case DescriptorType::StorageImage:
			return vk::DescriptorType::eStorageImage;
		case DescriptorType::CombinedImageSampler:
			return vk::DescriptorType::eCombinedImageSampler;
		case DescriptorType::AccelerationStructure:
			return vk::DescriptorType::eAccelerationStructureKHR;
		default:
			LOG_ERROR("Unhandled descriptor type enum: %d", static_cast<uint32_t>(descriptorType));
	}
	return vk::DescriptorType::eUniformBuffer;
}

vk::ShaderStageFlags mtd::EnumMapping::getShaderStage(ShaderStage shaderStage)
{
	vk::ShaderStageFlags flags{};

	if((shaderStage & ShaderStage::Vertex) != ShaderStage::None)
		flags |= vk::ShaderStageFlagBits::eVertex;
	if((shaderStage & ShaderStage::Fragment) != ShaderStage::None)
		flags |= vk::ShaderStageFlagBits::eFragment;
	if((shaderStage & ShaderStage::Compute) != ShaderStage::None)
		flags |= vk::ShaderStageFlagBits::eCompute;
	if((shaderStage & ShaderStage::RayGeneration) != ShaderStage::None)
		flags |= vk::ShaderStageFlagBits::eRaygenKHR;
	if((shaderStage & ShaderStage::ClosestHit) != ShaderStage::None)
		flags |= vk::ShaderStageFlagBits::eClosestHitKHR;
	if((shaderStage & ShaderStage::AnyHit) != ShaderStage::None)
		flags |= vk::ShaderStageFlagBits::eAnyHitKHR;
	if((shaderStage & ShaderStage::Miss) != ShaderStage::None)
		flags |= vk::ShaderStageFlagBits::eMissKHR;

	return flags;
}
