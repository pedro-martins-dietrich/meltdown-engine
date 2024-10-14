#pragma once

#include <glm/glm.hpp>

#include "../Descriptors/DescriptorSetHandler.hpp"
#include "ShaderModule.hpp"

namespace mtd
{
	class ComputePipeline
	{
		public:
			ComputePipeline(const vk::Device& device, const char* computeShaderFile);
			~ComputePipeline();

			ComputePipeline(const ComputePipeline&) = delete;
			ComputePipeline& operator=(const ComputePipeline&) = delete;

			DescriptorSetHandler& getDescriptorSetHandler(uint32_t index)
				{ return descriptorSetHandlers[index]; }

			// Starts the compute shader execution
			void dispatchCompute(const CommandHandler& commandHandler, glm::uvec3 workgroups) const;

		private:
			// Vulkan pipeline for compute shader
			vk::Pipeline computePipeline;
			// Compute pipeline layout
			vk::PipelineLayout pipelineLayout;

			// Compute shader
			ShaderModule computeShader;
			// Descriptor sets and their layouts
			std::vector<DescriptorSetHandler> descriptorSetHandlers;

			// Vulkan device reference
			const vk::Device& device;

			// Configures the descriptor set handlers to be used
			void createDescriptorSetLayouts();
			// Creates the layout for the compute pipeline
			void createPipelineLayout();
			// Creates the compute pipeline from the layout
			void createComputePipeline();
	};
}
