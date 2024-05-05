#pragma once

#include "../Device/Memory.hpp"

namespace mtd
{
	// Information about each descriptor
	struct DescriptorResources
	{
		Memory::Buffer descriptorBuffer;
		vk::DescriptorBufferInfo descriptorBufferInfo;
		void* descriptorBufferWriteLocation;
		vk::DescriptorType descriptorType;
	};

	// Handles the data to be sent to the GPU through descriptors
	class DescriptorSetHandler
	{
		public:
			DescriptorSetHandler
			(
				const vk::Device& device,
				const std::vector<vk::DescriptorSetLayoutBinding>& setLayoutBindings,
				uint32_t maxSets
			);
			~DescriptorSetHandler();

			DescriptorSetHandler(const DescriptorSetHandler&) = delete;
			DescriptorSetHandler& operator=(const DescriptorSetHandler&) = delete;

			DescriptorSetHandler(DescriptorSetHandler&&) noexcept;

			// Getters
			const vk::DescriptorSetLayout& getLayout() const { return descriptorSetLayout; }
			uint32_t getSetCount() const { return static_cast<uint32_t>(descriptorSets.size()); }
			vk::DescriptorSet& getSet(uint32_t index) { return descriptorSets[index]; }
			void* getBufferWriteLocation(uint32_t setIndex, uint32_t descriptorIndex) const
				{ return resourcesList[setIndex][descriptorIndex].descriptorBufferWriteLocation; }
			vk::DescriptorType getDescriptorType(uint32_t setIndex, uint32_t descriptorIndex) const
				{ return resourcesList[setIndex][descriptorIndex].descriptorType; }

			// Creates a descriptor and assings it to a descriptor set
			void createDescriptorResources
			(
				const Device& mtdDevice,
				vk::DeviceSize resourceSize,
				vk::BufferUsageFlags usageFlags,
				uint32_t setIndex,
				uint32_t resourceIndex
			);

			// Updates the descriptor set data
			void writeDescriptorSet(uint32_t setIndex);

		private:
			// Layout for the descriptor set
			vk::DescriptorSetLayout descriptorSetLayout;
			// Descriptor sets
			std::vector<vk::DescriptorSet> descriptorSets;

			// Data about the descriptors used in each descriptor set
			std::vector<std::vector<DescriptorResources>> resourcesList;
			// Write operations
			std::vector<vk::WriteDescriptorSet> writeOps;

			// Vulkan device reference
			const vk::Device& device;

			// Creates a descriptor set layout
			void createDescriptorSetLayout
			(
				const std::vector<vk::DescriptorSetLayoutBinding>& bindings
			);
	};
}
