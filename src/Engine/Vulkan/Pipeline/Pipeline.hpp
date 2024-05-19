#pragma once

#include "../Frame/Swapchain.hpp"
#include "ShaderModule.hpp"
#include "../Descriptors/DescriptorSetHandler.hpp"

namespace mtd
{
	// Vulkan graphics pipeline handler
	class Pipeline
	{
		public:
			Pipeline
			(
				const vk::Device& device,
				Swapchain& swapchain,
				DescriptorSetHandler* globalDescriptorSet
			);
			~Pipeline();

			Pipeline(const Pipeline&) = delete;
			Pipeline& operator=(const Pipeline&) = delete;

			// Getters
			const vk::Pipeline& getPipeline() const { return pipeline; }
			const vk::PipelineLayout& getLayout() const { return pipelineLayout; }
			const vk::RenderPass& getRenderPass() const { return renderPass; }
			PipelineSettings& getSettings() { return settings; }
			DescriptorSetHandler& getDescriptorSetHandler(uint32_t index)
				{ return descriptorSetHandlers[index]; }

			// Recreates the pipeline
			void recreate(Swapchain& swapchain, DescriptorSetHandler* globalDescriptorSet);

		private:
			// Vulkan graphics pipeline
			vk::Pipeline pipeline;
			// Pipeline layout
			vk::PipelineLayout pipelineLayout;
			// Pipeline render pass
			vk::RenderPass renderPass;

			// Descriptor sets and their layouts
			std::vector<DescriptorSetHandler> descriptorSetHandlers;

			// Customizable pipeline settings
			PipelineSettings settings;

			// Vulkan device reference
			const vk::Device& device;

			// Sets up default pipeline settings
			void configureDefaultSettings();

			// Creates the graphics pipeline
			void createPipeline(Swapchain& swapchain, DescriptorSetHandler* globalDescriptorSet);

			// Configures the descriptor set handlers to be used
			void createDescriptorSetLayouts();

			// Sets create infos for pipeline creation
			void setVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo) const;
			void setInputAssembly
			(
				vk::PipelineInputAssemblyStateCreateInfo& inputAssemblyInfo
			) const;
			void setVertexShader
			(
				std::vector<vk::PipelineShaderStageCreateInfo>& shaderStageInfos,
				const ShaderModule& vertexShaderModule
			) const;
			void setViewport
			(
				vk::PipelineViewportStateCreateInfo& viewportInfo,
				vk::Viewport& viewport,
				vk::Rect2D& scissor,
				const Swapchain& swapchain
			) const;
			void setRasterizer(vk::PipelineRasterizationStateCreateInfo& rasterizationInfo) const;
			void setFragmentShader
			(
				std::vector<vk::PipelineShaderStageCreateInfo>& shaderStageInfos,
				const ShaderModule& fragmentShaderModule
			) const;
			void setMultisampling(vk::PipelineMultisampleStateCreateInfo& multisampleInfo) const;
			void setDepthStencil(vk::PipelineDepthStencilStateCreateInfo& depthStencilInfo) const;
			void setColorBlending
			(
				vk::PipelineColorBlendStateCreateInfo& colorBlendInfo,
				vk::PipelineColorBlendAttachmentState& colorBlendAttachment
			) const;

			// Creates the layout for the pipeline
			void createPipelineLayout(DescriptorSetHandler* globalDescriptorSet);
			// Creates pipeline render pass
			void createRenderPass(Swapchain& swapchain);

			// Clears pipeline objects
			void destroy();
	};
}