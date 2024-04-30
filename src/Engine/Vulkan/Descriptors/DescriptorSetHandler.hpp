#pragma once

#include "../Device/Memory.hpp"

namespace mtd
{
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
			void* getBufferWriteLocation() const { return descriptorBufferWriteLocation; }
			const std::vector<vk::DescriptorType>& getDescriptorTypes() const
				{ return descriptorTypes; }

			// Creates a descriptor and assings it to a descriptor set
			void createDescriptorResources
			(
				const Device& mtdDevice,
				vk::DeviceSize resourceSize,
				vk::BufferUsageFlags usageFlags
			);

			// Updates the descriptor set data
			void writeDescriptorSet();

		private:
			// Layout for the descriptor set
			vk::DescriptorSetLayout descriptorSetLayout;
			// Descriptor set
			vk::DescriptorSet descriptorSet;

			// Descriptor resources
			Memory::Buffer descriptorBuffer;
			vk::DescriptorBufferInfo descriptorBufferInfo;
			void* descriptorBufferWriteLocation;

			// Write operation
			vk::WriteDescriptorSet writeOp;

			// Types of descriptor being used
			std::vector<vk::DescriptorType> descriptorTypes;

			// Vulkan device reference
			const vk::Device& device;

			// Creates a descriptor set layout
			void createDescriptorSetLayout
			(
				const std::vector<vk::DescriptorSetLayoutBinding>& bindings
			);
	};
}
