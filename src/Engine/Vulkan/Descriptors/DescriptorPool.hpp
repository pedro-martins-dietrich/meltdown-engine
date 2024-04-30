#pragma once

#include "DescriptorSetHandler.hpp"

namespace mtd
{
	struct PoolSizeData
	{
		uint32_t descriptorCount;
		vk::DescriptorType descriptorType;
	};

	// Manages a Vulkan descriptor pool
	class DescriptorPool
	{
		public:
			DescriptorPool(const vk::Device& device);
			~DescriptorPool();

			DescriptorPool(const DescriptorPool&) = delete;
			DescriptorPool& operator=(const DescriptorPool&) = delete;

			// Creates a descriptor pool
			void createDescriptorPool(const std::vector<PoolSizeData>& poolSizesInfo);

			// Allocates a descriptor set in the pool
			void allocateDescriptorSet(DescriptorSetHandler& descriptorSetHandler) const;

		private:
			// Pool to allocate descriptor sets
			vk::DescriptorPool descriptorPool;

			// Vulkan device reference
			const vk::Device& device;
	};
}
