#include <pch.hpp>
#include "DescriptorSetBuilder.hpp"

#include "../../../Utils/Logger.hpp"

namespace mtd
{
	// Translates the engine's descriptor type to the Vulkan equivalent
	static vk::DescriptorType mapDescriptorType(DescriptorType descriptorType);
	// Translates the engine's shader stage to the Vulkan equivalent
	static vk::ShaderStageFlags mapShaderStage(ShaderStage shaderStage);
}

void mtd::DescriptorSetBuilder::buildDescriptorSetLayout
(
	std::vector<vk::DescriptorSetLayoutBinding>& bindings,
	std::vector<DescriptorInfo>& descriptorInfos,
	std::unordered_map<vk::DescriptorType, uint32_t>& descriptorTypeCount
)
{
	bindings.resize(descriptorInfos.size());
	for(uint32_t i = 0; i < descriptorInfos.size(); i++)
	{
		const vk::DescriptorType descriptorType = mapDescriptorType(descriptorInfos[i].descriptorType);
		descriptorTypeCount[descriptorType] += descriptorInfos[i].descriptorCount;

		bindings[i].binding = i;
		bindings[i].descriptorType = descriptorType;
		bindings[i].descriptorCount = descriptorInfos[i].descriptorCount;
		bindings[i].stageFlags = mapShaderStage(descriptorInfos[i].shaderStage);
		bindings[i].pImmutableSamplers = nullptr;
	}
}

vk::DescriptorType mtd::mapDescriptorType(DescriptorType descriptorType)
{
	switch(descriptorType)
	{
		case DescriptorType::UniformBuffer:
			return vk::DescriptorType::eUniformBuffer;
		case DescriptorType::StorageImage:
			return vk::DescriptorType::eStorageImage;
		default:
			LOG_ERROR("Failed to map descriptor type. Used descriptor type: %d", descriptorType);
	}
	return vk::DescriptorType::eUniformBuffer;
}

vk::ShaderStageFlags mtd::mapShaderStage(ShaderStage shaderStage)
{
	switch(shaderStage)
	{
		case ShaderStage::Vertex:
			return vk::ShaderStageFlagBits::eVertex;
		case ShaderStage::Fragment:
			return vk::ShaderStageFlagBits::eFragment;
		case ShaderStage::Vertex_Fragment:
			return vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
		case ShaderStage::Compute:
			return vk::ShaderStageFlagBits::eCompute;
		case ShaderStage::RayGeneration:
			return vk::ShaderStageFlagBits::eRaygenKHR;
		case ShaderStage::ClosestHit:
			return vk::ShaderStageFlagBits::eClosestHitKHR;
		case ShaderStage::AnyHit:
			return vk::ShaderStageFlagBits::eAnyHitKHR;
		case ShaderStage::Miss:
			return vk::ShaderStageFlagBits::eMissKHR;
		case ShaderStage::RayGeneration_ClosestHit:
			return vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR;
		case ShaderStage::RayGeneration_AnyHit:
			return vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eAnyHitKHR;
		case ShaderStage::RayGeneration_Miss:
			return vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eMissKHR;
		case ShaderStage::ClosestHit_AnyHit:
			return vk::ShaderStageFlagBits::eClosestHitKHR | vk::ShaderStageFlagBits::eAnyHitKHR;
		case ShaderStage::ClosestHit_Miss:
			return vk::ShaderStageFlagBits::eClosestHitKHR | vk::ShaderStageFlagBits::eMissKHR;
		case ShaderStage::AnyHit_Miss:
			return vk::ShaderStageFlagBits::eAnyHitKHR | vk::ShaderStageFlagBits::eMissKHR;
		case ShaderStage::RayGeneration_ClosestHit_AnyHit:
			return vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR
				| vk::ShaderStageFlagBits::eAnyHitKHR;
		case ShaderStage::RayGeneration_ClosestHit_Miss:
			return vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR
				| vk::ShaderStageFlagBits::eMissKHR;
		case ShaderStage::RayGeneration_AnyHit_Miss:
			return vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eAnyHitKHR
				| vk::ShaderStageFlagBits::eMissKHR;
		case ShaderStage::ClosestHit_AnyHit_Miss:
			return vk::ShaderStageFlagBits::eClosestHitKHR | vk::ShaderStageFlagBits::eAnyHitKHR
				| vk::ShaderStageFlagBits::eMissKHR;
		case ShaderStage::RayGeneration_ClosestHit_AnyHit_Miss:
			return vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR
				| vk::ShaderStageFlagBits::eAnyHitKHR | vk::ShaderStageFlagBits::eMissKHR;
		default:
			LOG_ERROR("Failed to map shader stage. Shader stage requested: %d", shaderStage);
	}
	return vk::ShaderStageFlagBits::eFragment;
}
