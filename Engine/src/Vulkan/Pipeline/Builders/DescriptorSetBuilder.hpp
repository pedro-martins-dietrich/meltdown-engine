#pragma once

#include <meltdown/structs.hpp>

namespace mtd::DescriptorSetBuilder
{
	void buildDescriptorSetLayout
	(
		std::vector<vk::DescriptorSetLayoutBinding>& bindings,
		std::vector<DescriptorInfo>& descriptorInfos,
		std::unordered_map<vk::DescriptorType, uint32_t>& descriptorTypeCount
	);
}
