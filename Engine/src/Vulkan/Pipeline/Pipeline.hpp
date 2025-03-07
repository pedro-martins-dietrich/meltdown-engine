#pragma once

#include "ShaderModule.hpp"
#include "../Descriptors/DescriptorPool.hpp"

#include "Builders/PipelineMapping.hpp"

namespace mtd
{
	// General Vulkan pipeline handler
	template<typename PipelineInfoType = PipelineInfo>
	class Pipeline
	{
		static_assert
		(
			std::is_base_of_v<PipelineInfo, PipelineInfoType>, "PipelineInfoType must be derived from PipelineInfo."
		);

		public:
			Pipeline(const vk::Device& device, const PipelineInfoType& info)
				: device{device}, pipeline{nullptr}, pipelineLayout{nullptr}, info{info}
			{}

			~Pipeline()
			{
				device.destroyPipeline(pipeline);
				device.destroyPipelineLayout(pipelineLayout);
			}
			
			Pipeline(const Pipeline&) = delete;
			Pipeline& operator=(const Pipeline&) = delete;

			Pipeline(Pipeline&& other) noexcept
				: device{other.device},
				pipeline{std::move(other.pipeline)},
				pipelineLayout{std::move(other.pipelineLayout)},
				shaders{std::move(other.shaders)},
				descriptorSetHandlers{std::move(other.descriptorSetHandlers)},
				descriptorTypeCount{std::move(other.descriptorTypeCount)},
				info{std::move(other.info)}
			{
				other.pipeline = nullptr;
				other.pipelineLayout = nullptr;
			}

			// Getters
			const vk::Pipeline& getPipeline() const { return pipeline; }
			const vk::PipelineLayout& getLayout() const { return pipelineLayout; }
			const std::string& getName() const { return info.pipelineName; }
			DescriptorSetHandler& getDescriptorSetHandler(uint32_t set) { return descriptorSetHandlers[set]; }
			const std::unordered_map<vk::DescriptorType, uint32_t>& getDescriptorTypeCount() const
				{ return descriptorTypeCount; }

			// Allocates user descriptor set data in the descriptor pool
			void configureUserDescriptorData(const Device& mtdDevice, const DescriptorPool& pool)
			{
				if(descriptorSetHandlers.size() < 2) return;

				DescriptorSetHandler& descriptorSetHandler = descriptorSetHandlers[1];
				descriptorSetHandler.defineDescriptorSetsAmount(1);
				pool.allocateDescriptorSet(descriptorSetHandler);

				for(uint32_t binding = 0; binding < info.descriptorSetInfo.size(); binding++)
				{
					const DescriptorInfo& bindingInfo = info.descriptorSetInfo[binding];
					descriptorSetHandler.createDescriptorResources
					(
						mtdDevice,
						bindingInfo.totalDescriptorSize,
						PipelineMapping::mapBufferUsageFlags(bindingInfo.descriptorType),
						0, binding
					);
				}
				descriptorSetHandler.writeDescriptorSet(0);
			}

			// Updates the user descriptor data for the specified binding
			void updateDescriptorData(uint32_t bindingIndex, const void* data) const
			{
				if(descriptorSetHandlers.size() < 2 || descriptorSetHandlers[1].getSetCount() <= bindingIndex) return;

				descriptorSetHandlers[1].updateDescriptorData
				(
					0, bindingIndex, data, info.descriptorSetInfo[bindingIndex].totalDescriptorSize
				);
			}

		protected:
			// Vulkan graphics pipeline
			vk::Pipeline pipeline;
			// Pipeline layout
			vk::PipelineLayout pipelineLayout;

			// Shader modules used in the pipeline
			std::vector<ShaderModule> shaders;
			// Descriptor sets and their layouts
			std::vector<DescriptorSetHandler> descriptorSetHandlers;
			// Required descriptor count for each descriptor type of the current pipeline
			std::unordered_map<vk::DescriptorType, uint32_t> descriptorTypeCount;

			// Pipeline specific configurations
			PipelineInfoType info;

			// Vulkan device reference
			const vk::Device& device;
	};
}
