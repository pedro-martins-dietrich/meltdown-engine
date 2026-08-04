#pragma once

#include <vulkan/vulkan.hpp>

#include <meltdown/enums.hpp>

// Responsible for converting the engine enums to their Vulkan counterpart
namespace mtd::EnumMapping
{
    // Finds the Vulkan equivalent flags for the GPU buffer usages
    vk::BufferUsageFlags getBufferUsage(GpuBufferType bufferType);
    // Finds the Vulkan equivalent flags for the GPU memory properties
    vk::MemoryPropertyFlags getMemoryProperties(GpuMemoryUsage memoryUsage);
    // Decides the defalt value for the GPU memory usage based on the GPU buffer type
    GpuMemoryUsage defaultMemoryUsage(GpuBufferType bufferType);

    // Finds the Vulkan equivalent enum for the descriptor type
    vk::DescriptorType getDescriptorType(DescriptorType descriptorType);
    // Finds the Vulkan equivalent flags for the shader stage
    vk::ShaderStageFlags getShaderStage(ShaderStage shaderStage);
}
