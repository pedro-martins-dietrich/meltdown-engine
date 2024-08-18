#pragma once

#include "../Frame/Swapchain.hpp"
#include "../Descriptors/DescriptorSetHandler.hpp"
#include "ShaderModule.hpp"

namespace mtd
{
	// Vulkan graphics pipeline handler
	class Pipeline
	{
		public:
			Pipeline
			(
				const vk::Device& device,
				PipelineType type,
				Swapchain& swapchain,
				DescriptorSetHandler* globalDescriptorSet
			);
			~Pipeline();

			Pipeline(const Pipeline&) = delete;
			Pipeline& operator=(const Pipeline&) = delete;

			// Getters
			const vk::Pipeline& getPipeline() const { return pipeline; }
			const vk::PipelineLayout& getLayout() const { return pipelineLayout; }
			PipelineSettings& getSettings() { return settings; }
			DescriptorSetHandler& getDescriptorSetHandler(uint32_t index)
				{ return descriptorSetHandlers[index]; }

			// Recreates the pipeline
			void recreate
			(
				Swapchain& swapchain,
				DescriptorSetHandler* globalDescriptorSet
			);

			// Binds the pipeline to the command buffer
			void bind(const vk::CommandBuffer& commandBuffer) const;
			// Binds per mesh descriptors
			void bindDescriptors(const vk::CommandBuffer& commandBuffer, uint32_t index) const;

		private:
			// Pipeline type
			PipelineType type;

			// Vulkan graphics pipeline
			vk::Pipeline pipeline;
			// Pipeline layout
			vk::PipelineLayout pipelineLayout;

			// Shader modules used in the pipeline
			std::vector<ShaderModule> shaders;
			// Descriptor sets and their layouts
			std::vector<DescriptorSetHandler> descriptorSetHandlers;

			// Customizable pipeline settings
			PipelineSettings settings;

			// Vulkan device reference
			const vk::Device& device;

			// Sets up default pipeline settings
			void configureDefaultSettings();

			// Creates the graphics pipeline
			void createPipeline
			(
				Swapchain& swapchain,
				DescriptorSetHandler* globalDescriptorSet
			);

			// Configures the descriptor set handlers to be used
			void createDescriptorSetLayouts();

			// Sets create infos for pipeline creation
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

			// Creates the layout for the pipeline
			void createPipelineLayout(DescriptorSetHandler* globalDescriptorSet);

			// Clears pipeline objects
			void destroy();
	};
}