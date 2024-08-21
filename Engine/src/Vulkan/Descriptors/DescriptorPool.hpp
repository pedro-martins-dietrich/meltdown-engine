#pragma once

#include "DescriptorSetHandler.hpp"

namespace mtd
{
	// Information about the descriptors to be allocated in the pool
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

			// Getter
			const vk::DescriptorPool& getDescriptorPool() const { return descriptorPool; }

			// Creates a descriptor pool
			void createDescriptorPool
			(
				const std::vector<PoolSizeData>& poolSizesInfo,
				vk::DescriptorPoolCreateFlags flags = vk::DescriptorPoolCreateFlags()
			);

			// Allocates a descriptor set in the pool
			void allocateDescriptorSet(DescriptorSetHandler& descriptorSetHandler) const;

		private:
			// Pool to allocate descriptor sets
			vk::DescriptorPool descriptorPool;

			// Vulkan device reference
			const vk::Device& device;
	};
}
