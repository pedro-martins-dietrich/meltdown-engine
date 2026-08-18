#pragma once

#include <vulkan/vulkan.hpp>

namespace mtd
{
    // Available Vulkan samplers
    enum class SamplerType
    {
        Linear,
        Nearest
    };

    // Global manager for Vulkan samplers
    namespace SamplerManager
    {
        // Creates all the samplers that can be used
        void createSamplers(const vk::Device& vulkanDevice);
        // Destroys all existing samplers
        void destroySamplers(const vk::Device& vulkanDevice);

        // Fetches the Vulkan sampler, creating it if required
        vk::Sampler getSampler(SamplerType type);
    };
}
