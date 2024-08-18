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
		vk::DescriptorImageInfo descriptorImageInfo;
	};

	// Handles the data to be sent to the GPU through descriptors
	class DescriptorSetHandler
	{
		public:
			DescriptorSetHandler
			(
				const vk::Device& device,
				const std::vector<vk::DescriptorSetLayoutBinding>& setLayoutBindings
			);
			~DescriptorSetHandler();

			DescriptorSetHandler(const DescriptorSetHandler&) = delete;
			DescriptorSetHandler& operator=(const DescriptorSetHandler&) = delete;

			DescriptorSetHandler(DescriptorSetHandler&&) noexcept;

			// Getters
			const vk::DescriptorSetLayout& getLayout() const { return descriptorSetLayout; }
			uint32_t getSetCount() const { return static_cast<uint32_t>(descriptorSets.size()); }
			vk::DescriptorSet& getSet(uint32_t index) { return descriptorSets[index]; }
			const vk::DescriptorSet& getSet(uint32_t index) const { return descriptorSets[index]; }
			std::vector<vk::DescriptorSet>& getSets() { return descriptorSets; }
			void* getBufferWriteLocation(uint32_t setIndex, uint32_t descriptorIndex) const
				{ return resourcesList[setIndex][descriptorIndex].descriptorBufferWriteLocation; }
			vk::DescriptorType getDescriptorType(uint32_t descriptorIndex) const
				{ return descriptorTypes[descriptorIndex]; }

			// Defines how many descriptor sets can be associated with the descriptor set layout
			void defineDescriptorSetsAmount(uint32_t setsAmount);

			// Creates a descriptor and assings it to a descriptor set
			void createDescriptorResources
			(
				const Device& mtdDevice,
				vk::DeviceSize resourceSize,
				vk::BufferUsageFlags usageFlags,
				uint32_t setIndex,
				uint32_t resourceIndex
			);
			// Creates the resources for an image descriptor
			void createImageDescriptorResources
			(
				uint32_t setIndex,
				uint32_t resourceIndex,
				const vk::DescriptorImageInfo& descriptorImageInfo
			);

			// Updates the descriptor set data
			void writeDescriptorSet(uint32_t setIndex);

		private:
			// Layout for the descriptor set
			vk::DescriptorSetLayout descriptorSetLayout;
			// Descriptor sets
			std::vector<vk::DescriptorSet> descriptorSets;

			// Descriptor type for each binding in the set layout
			std::vector<vk::DescriptorType> descriptorTypes;

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
