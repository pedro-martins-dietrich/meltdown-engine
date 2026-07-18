#pragma once

#include "../Device/GpuBuffer.hpp"

namespace mtd
{
	// Information about each descriptor
	struct DescriptorResources
	{
		std::unique_ptr<GpuBuffer> descriptorBuffer;
		vk::DescriptorBufferInfo descriptorBufferInfo;
		std::vector<vk::DescriptorImageInfo> descriptorImagesInfo;
	};

	// Manages the accessibility of GPU data in the shaders through descriptors
	class DescriptorSetHandler
	{
		public:
			DescriptorSetHandler
			(
				const vk::Device& device,
				const std::vector<vk::DescriptorSetLayoutBinding>& setLayoutBindings,
				const vk::DescriptorSetLayoutBindingFlagsCreateInfo* pFlags = nullptr
			);
			~DescriptorSetHandler();

			DescriptorSetHandler(const DescriptorSetHandler&) = delete;
			DescriptorSetHandler& operator=(const DescriptorSetHandler&) = delete;

			DescriptorSetHandler(DescriptorSetHandler&&) noexcept;

			// Getters
			const vk::DescriptorSetLayout& getLayout() const { return descriptorSetLayout; }
			const vk::DescriptorSet& getSet() const { return descriptorSet; }
			vk::DescriptorSet& getSet() { return descriptorSet; }
			uint32_t getBindingCount() const { return static_cast<uint32_t>(writeOperations.size()); }
			vk::DescriptorType getDescriptorType(uint32_t binding) const
				{ return writeOperations[binding].descriptorType; }

			// Creates a descriptor, assigning it to a set and returning the buffer write location
			void createDescriptorResources
			(
				const Device& mtdDevice, vk::DeviceSize resourceSize, vk::BufferUsageFlags usageFlags, uint32_t binding
			);
			// Creates the resources for an image descriptor
			void createImageDescriptorResources(uint32_t binding, const vk::DescriptorImageInfo& descriptorImageInfo);
			// Creates the resources for a vector of images descriptor
			void createImagesDescriptorResources
			(
				uint32_t binding, std::vector<vk::DescriptorImageInfo>& descriptorImagesInfo
			);
			// Assigns an external GPU buffer to a descriptor
			void assignExternalResourcesToDescriptor
			(
				uint32_t binding, const GpuBuffer& buffer, const void* pNext = nullptr
			);
			// Assigns an external GPU buffer to a descriptor
			void assignBuffer
			(
				uint32_t binding, const vk::DescriptorBufferInfo& bufferInfo, const void* pNext = nullptr
			);

			// Updates all the descriptors in the set
			void writeDescriptorSet();
			// Updates a specific descriptor in the set
			void writeDescriptor(uint32_t binding);

			// Updates the descriptor data
			void updateDescriptorData(uint32_t binding, const void* data, vk::DeviceSize dataSize) const;

		private:
			// Layout for the descriptor set
			vk::DescriptorSetLayout descriptorSetLayout;
			// Descriptor set
			vk::DescriptorSet descriptorSet;

			// Write operations for each descriptor in the set
			std::vector<vk::WriteDescriptorSet> writeOperations;
			// Data about the descriptors used in each descriptor set
			std::vector<DescriptorResources> resourcesList;

			// Vulkan device reference
			const vk::Device& device;

			// Creates a descriptor set layout
			void createDescriptorSetLayout
			(
				const std::vector<vk::DescriptorSetLayoutBinding>& bindings,
				const vk::DescriptorSetLayoutBindingFlagsCreateInfo* pFlags
			);
	};
}
