#include <pch.hpp>
#include "DescriptorSetBuilder.hpp"

#include "../../EnumMapping/EnumMapping.hpp"
#include "../../../Utils/Logger.hpp"

void mtd::DescriptorSetBuilder::buildDescriptorSetLayout
(
	std::vector<vk::DescriptorSetLayoutBinding>& bindings,
	std::vector<DescriptorInfo>& descriptorInfos,
	std::unordered_map<vk::DescriptorType, uint32_t>& descriptorTypeCount
)
{
	bindings.resize(descriptorInfos.size());
	for(uint32_t i = 0U; i < descriptorInfos.size(); i++)
	{
		const vk::DescriptorType descriptorType = EnumMapping::getDescriptorType(descriptorInfos[i].descriptorType);
		descriptorTypeCount[descriptorType] += descriptorInfos[i].descriptorCount;

		bindings[i].binding = i;
		bindings[i].descriptorType = descriptorType;
		bindings[i].descriptorCount = descriptorInfos[i].descriptorCount;
		bindings[i].stageFlags = EnumMapping::getShaderStage(descriptorInfos[i].shaderStage);
		bindings[i].pImmutableSamplers = nullptr;
	}
}
