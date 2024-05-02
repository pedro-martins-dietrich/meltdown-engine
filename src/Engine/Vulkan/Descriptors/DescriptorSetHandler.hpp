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
				const std::vector<vk::DescriptorSetLayoutBinding>& setLayoutBindings
			);
			~DescriptorSetHandler();

			DescriptorSetHandler(const DescriptorSetHandler&) = delete;
			DescriptorSetHandler& operator=(const DescriptorSetHandler&) = delete;

			DescriptorSetHandler(DescriptorSetHandler&&) noexcept;

			// Getters
			const vk::DescriptorSetLayout& getLayout() const { return descriptorSetLayout; }
			vk::DescriptorSet& getSet() { return descriptorSet; }
			void* getBufferWriteLocation(uint32_t index) const
				{ return resourcesList[index].descriptorBufferWriteLocation; }
			vk::DescriptorType getDescriptorType(uint32_t index) const
				{ return resourcesList[index].descriptorType; }

			// Creates a descriptor and assings it to a descriptor set
			void createDescriptorResources
			(
				const Device& mtdDevice,
				vk::DeviceSize resourceSize,
				vk::BufferUsageFlags usageFlags,
				uint32_t resourceIndex
			);

			// Updates the descriptor set data
			void writeDescriptorSet();

		private:
			// Layout for the descriptor set
			vk::DescriptorSetLayout descriptorSetLayout;
			// Descriptor set
			vk::DescriptorSet descriptorSet;

			// Data about the descriptors used in the descriptor set
			std::vector<DescriptorResources> resourcesList;
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
