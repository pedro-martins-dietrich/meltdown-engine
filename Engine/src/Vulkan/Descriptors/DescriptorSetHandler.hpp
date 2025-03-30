#pragma once

#include "../Device/GpuBuffer.hpp"

namespace mtd
{
	// Information about each descriptor
	struct DescriptorResources
	{
		std::unique_ptr<GpuBuffer> descriptorBuffer;
		vk::DescriptorBufferInfo descriptorBufferInfo;
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
			std::vector<vk::DescriptorSet>& getSets() { return descriptorSets; }
			vk::DescriptorSet& getSet(uint32_t swappableSet) { return descriptorSets[swappableSet]; }
			const vk::DescriptorSet& getSet(uint32_t swappableSet) const { return descriptorSets[swappableSet]; }
			vk::DescriptorType getDescriptorType(uint32_t binding) const { return descriptorTypes[binding]; }

			// Defines how many descriptor sets can be associated with the descriptor set layout
			void defineDescriptorSetsAmount(uint32_t swappableSetsAmount);

			// Creates a descriptor, assigning it to a set and returning the buffer write location
			void createDescriptorResources
			(
				const Device& mtdDevice,
				vk::DeviceSize resourceSize,
				vk::BufferUsageFlags usageFlags,
				uint32_t swappableSetIndex,
				uint32_t binding
			);
			// Creates the resources for an image descriptor
			void createImageDescriptorResources
			(
				uint32_t swappableSetIndex,
				uint32_t binding,
				const vk::DescriptorImageInfo& descriptorImageInfo
			);
			// Assigns an external GPU buffer as a descriptor
			void assignExternalResourcesToDescriptor
			(
				uint32_t swappableSetIndex, uint32_t binding, const GpuBuffer& buffer
			);

			// Updates the descriptor set write data
			void writeDescriptorSet(uint32_t swappableSetIndex);

			// Updates the descriptor data
			void updateDescriptorData
			(
				uint32_t swappableSetIndex, uint32_t binding, const void* data, vk::DeviceSize dataSize
			) const;

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
			void createDescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings);
	};
}
