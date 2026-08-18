#include <pch.hpp>
#include "SamplerManager.hpp"

#include "../../Utils/Logger.hpp"

namespace mtd::SamplerManager
{
    static std::unordered_map<mtd::SamplerType, vk::Sampler> samplers;
}

void mtd::SamplerManager::createSamplers(const vk::Device& vulkanDevice)
{
    vk::Sampler sampler;

    vk::SamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.flags = vk::SamplerCreateFlags();
	samplerCreateInfo.magFilter = vk::Filter::eLinear;
	samplerCreateInfo.minFilter = vk::Filter::eLinear;
	samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = vk::False;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	samplerCreateInfo.compareEnable = vk::False;
	samplerCreateInfo.compareOp = vk::CompareOp::eAlways;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerCreateInfo.unnormalizedCoordinates = vk::False;

    // Linear
    vk::Result result = vulkanDevice.createSampler(&samplerCreateInfo, nullptr, &sampler);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create image sampler. Vulkan result: %d", result);
    samplers[SamplerType::Linear] = sampler;

    // Nearest
    samplerCreateInfo.magFilter = vk::Filter::eNearest;
	samplerCreateInfo.minFilter = vk::Filter::eNearest;
    result = vulkanDevice.createSampler(&samplerCreateInfo, nullptr, &sampler);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create image sampler. Vulkan result: %d", result);
    samplers[SamplerType::Nearest] = sampler;
}

void mtd::SamplerManager::destroySamplers(const vk::Device& vulkanDevice)
{
    for(auto& [type, sampler]: samplers)
    {
        if(sampler)
            vulkanDevice.destroySampler(sampler);
    }
    samplers.clear();
}

vk::Sampler mtd::SamplerManager::getSampler(SamplerType type)
{
    assert(samplers.find(type) != samplers.end() && "The samplers must be created before fetching them.");
    return samplers.at(type);
}
