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
		void* descriptorBufferWriteLocation;
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
			vk::DescriptorSet& getSet(uint32_t swappableSet) { return descriptorSets[swappableSet]; }
			const vk::DescriptorSet& getSet(uint32_t swappableSet) const { return descriptorSets[swappableSet]; }
			std::vector<vk::DescriptorSet>& getSets() { return descriptorSets; }
			vk::DescriptorType getDescriptorType(uint32_t binding) const { return descriptorTypes[binding]; }
			void* getWriteLocation(uint32_t swappableSet, uint32_t binding) const
				{ return resourcesList[swappableSet][binding].descriptorBufferWriteLocation; }

			// Defines how many descriptor sets can be associated with the descriptor set layout
			void defineDescriptorSetsAmount(uint32_t setsAmount);

			// Creates a descriptor, assigning it to a set and returning the buffer write location
			void* createDescriptorResources
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

			// Deletes all resources in GPU memory
			void clearResources();

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
