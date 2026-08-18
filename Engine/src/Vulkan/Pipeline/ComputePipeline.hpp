#pragma once

#include <meltdown/event.hpp>

#include "Pipeline.hpp"

namespace mtd
{
	class ComputePipeline : public Pipeline<ComputePipelineInfo>
	{
		public:
			ComputePipeline
			(
				const Device& mtdDevice,
				const DescriptorManager& descriptorManager,
				const ComputePipelineInfo& info,
				vk::Extent2D swapchainExtent
			);
			~ComputePipeline() = default;

			ComputePipeline(const ComputePipeline&) = delete;
			ComputePipeline& operator=(const ComputePipeline&) = delete;

			ComputePipeline(ComputePipeline&& other) noexcept;

			// Setter
			void setInstanceCount(uint32_t instanceCount) const { pushConstantData.instanceCount = instanceCount; }

			// Starts the compute shader execution
			void dispatchCompute(const vk::CommandBuffer& commandBuffer) const;

			// Configures the render target image descriptor
			void configurePipelineDescriptorSet();

			// Configures the render target image as a descriptor for another descriptor set
			void shareRenderTargetImageDescriptor
			(
				DescriptorSetHandler& descriptorSetHandler, uint32_t binding
			) const;

			// Resizes the render target image if needed
			void resize(const Device& mtdDevice, vk::Extent2D swapchainExtent);

		private:
			// Output image
			Image outputImage;
			// Images available to the compute shader
			std::vector<Image> images;
			// Current render target image index
			uint32_t currentRenderTargerImageIndex = 0U;
			// Flag to recreate image on window resize
			bool windowResolutionDependant = false;

			// Push constant data for the compute shader
			mutable ComputeShaderPushConstantData pushConstantData;

			// Event callback handle for resetting frame accumulation
			EventCallbackHandle resetAccumulationCallbackHandle;

			// Loads the compute shader module
			void loadShaderModule();
			// Configures the descriptor set handlers to be used
			void createDescriptorSetLayouts();

			// Creates the layout for the compute pipeline
			void createPipelineLayout();
			// Creates the compute pipeline from the layout
			void createComputePipeline();

			// Creates the storage images for the ray trace rendering
			void createStorageImages(const Device& mtdDevice, vk::Extent2D swapchainExtent);
			// Sets up the frame accumulation reset event callback
			void setEventCallback();
	};
}
