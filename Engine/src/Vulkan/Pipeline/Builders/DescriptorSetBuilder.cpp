#include <pch.hpp>
#include "DescriptorSetBuilder.hpp"

#include "../../../Utils/Logger.hpp"

static vk::DescriptorType mapDescriptorType(mtd::DescriptorType descriptorType);
static vk::ShaderStageFlags mapShaderStage(mtd::ShaderStage shaderStage);

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

vk::DescriptorType mapDescriptorType(mtd::DescriptorType descriptorType)
{
	switch(descriptorType)
	{
		case mtd::DescriptorType::UniformBuffer:
			return vk::DescriptorType::eUniformBuffer;
		default:
			mtd::LOG_ERROR("Failed to map descriptor type. Used descriptor type: %d", descriptorType);
	}
	return vk::DescriptorType::eUniformBuffer;
}

vk::ShaderStageFlags mapShaderStage(mtd::ShaderStage shaderStage)
{
	switch(shaderStage)
	{
		case mtd::ShaderStage::Vertex:
			return vk::ShaderStageFlagBits::eVertex;
		case mtd::ShaderStage::Fragment:
			return vk::ShaderStageFlagBits::eFragment;
		case mtd::ShaderStage::VertexAndFragment:
			return vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
		default:
			mtd::LOG_ERROR("Failed to map shader stage. Shader stage used: %d", shaderStage);
	}
	return vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
}
