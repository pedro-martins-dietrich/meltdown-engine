#pragma once

#include "../Frame/Swapchain.hpp"
#include "ShaderModule.hpp"

namespace mtd
{
	// Vulkan graphics pipeline handler
	class Pipeline
	{
		public:
			Pipeline(const vk::Device& device, Swapchain& swapchain);
			~Pipeline();

			Pipeline(const Pipeline&) = delete;
			Pipeline& operator=(const Pipeline&) = delete;

			// Getters
			const vk::Pipeline& getPipeline() const { return pipeline; }
			const vk::RenderPass& getRenderPass() const { return renderPass; }

		private:
			// Vulkan graphics pipeline
			vk::Pipeline pipeline;
			// Pipeline layout
			vk::PipelineLayout pipelineLayout;
			// Pipeline render pass
			vk::RenderPass renderPass;

			// Vulkan device reference
			const vk::Device& device;

			// Creates the graphics pipeline
			void createPipeline(Swapchain& swapchain);

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
			void createPipelineLayout();
			// Creates pipeline render pass
			void createRenderPass(Swapchain& swapchain);
	};
}