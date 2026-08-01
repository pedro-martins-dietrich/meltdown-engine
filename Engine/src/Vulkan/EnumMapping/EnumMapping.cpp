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

    if((bufferType & GpuBufferType::TransferSource) != GpuBufferType::None)
        bufferUsage |= vk::BufferUsageFlagBits::eTransferSrc;
    if((bufferType & GpuBufferType::TransferDestination) != GpuBufferType::None)
        bufferUsage |= vk::BufferUsageFlagBits::eTransferDst;

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
            return vk::MemoryPropertyFlagBits::eDeviceLocal;
    }
    return vk::MemoryPropertyFlagBits::eDeviceLocal;
}

mtd::GpuMemoryUsage mtd::EnumMapping::defaultMemoryUsage(GpuBufferType bufferType)
{
    if((bufferType & GpuBufferType::Uniform) != GpuBufferType::None)
        return GpuMemoryUsage::CpuUpload;
    if((bufferType & GpuBufferType::Storage) != GpuBufferType::None)
        return GpuMemoryUsage::GpuOnly;

    return GpuMemoryUsage::GpuOnly;
}
