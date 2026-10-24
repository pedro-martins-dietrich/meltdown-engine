#include <pch.hpp>
#include "EnumMapping.hpp"

#include "../../Utils/Logger.hpp"

vk::BufferUsageFlags mtd::EnumMapping::getBufferUsage(GpuBufferType bufferType)
{
	vk::BufferUsageFlags bufferUsage{};

	if(!!(bufferType & GpuBufferType::Uniform))
		bufferUsage |= vk::BufferUsageFlagBits::eUniformBuffer;
	if(!!(bufferType & GpuBufferType::Storage))
		bufferUsage |= vk::BufferUsageFlagBits::eStorageBuffer;
	if(!!(bufferType & GpuBufferType::Vertex))
		bufferUsage |= (vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer);
	if(!!(bufferType & GpuBufferType::Index))
		bufferUsage |= (vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eStorageBuffer);
	if(!!(bufferType |= GpuBufferType::AccelerationStructure))
		bufferUsage |=
		(
			vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
			| vk::BufferUsageFlagBits::eShaderDeviceAddress
		);
	if(!!(bufferType & GpuBufferType::AccelerationStructureInput))
		bufferUsage |=
		(
			vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
			| vk::BufferUsageFlagBits::eShaderDeviceAddress
		);
	if(!!(bufferType & GpuBufferType::ShaderBindingTable))
		bufferUsage |=
		(
			vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress
			| vk::BufferUsageFlagBits::eTransferDst
		);

	if(!!(bufferType & GpuBufferType::TransferSource))
		bufferUsage |= vk::BufferUsageFlagBits::eTransferSrc;
	if(!!(bufferType & GpuBufferType::TransferDestination))
		bufferUsage |= vk::BufferUsageFlagBits::eTransferDst;
	if(!!(bufferType & GpuBufferType::DeviceAddress))
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
	if(!!(bufferType & GpuBufferType::Uniform) || !!(bufferType & GpuBufferType::AccelerationStructureInput))
		return GpuMemoryUsage::CpuUpload;

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

	if(!!(shaderStage & ShaderStage::Vertex))
		flags |= vk::ShaderStageFlagBits::eVertex;
	if(!!(shaderStage & ShaderStage::Fragment))
		flags |= vk::ShaderStageFlagBits::eFragment;
	if(!!(shaderStage & ShaderStage::Compute))
		flags |= vk::ShaderStageFlagBits::eCompute;
	if(!!(shaderStage & ShaderStage::RayGeneration))
		flags |= vk::ShaderStageFlagBits::eRaygenKHR;
	if(!!(shaderStage & ShaderStage::ClosestHit))
		flags |= vk::ShaderStageFlagBits::eClosestHitKHR;
	if(!!(shaderStage & ShaderStage::AnyHit))
		flags |= vk::ShaderStageFlagBits::eAnyHitKHR;
	if(!!(shaderStage & ShaderStage::Miss))
		flags |= vk::ShaderStageFlagBits::eMissKHR;

	return flags;
}
