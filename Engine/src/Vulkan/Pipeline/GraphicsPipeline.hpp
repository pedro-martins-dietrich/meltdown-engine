#pragma once

#include <meltdown/structs.hpp>

#include "ShaderModule.hpp"
#include "../Descriptors/DescriptorPool.hpp"

namespace mtd
{
	// Vulkan graphics (rasterization) pipeline handler
	class GraphicsPipeline
	{
		public:
			GraphicsPipeline
			(
				const vk::Device& device,
				const GraphicsPipelineInfo& info,
				const vk::DescriptorSetLayout& globalDescriptorSetLayout,
				vk::Extent2D extent,
				vk::RenderPass renderPass
			);
			~GraphicsPipeline();

			GraphicsPipeline(const GraphicsPipeline&) = delete;
			GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

			GraphicsPipeline(GraphicsPipeline&& other) noexcept;

			// Getters
			const vk::Pipeline& getPipeline() const { return pipeline; }
			const vk::PipelineLayout& getLayout() const { return pipelineLayout; }
			const std::string& getName() const { return info.pipelineName; }
			int32_t getTargetFramebuffer() const { return info.targetFramebufferIndex; }
			MeshType getAssociatedMeshType() const { return info.associatedMeshType; }
			DescriptorSetHandler& getDescriptorSetHandler(uint32_t set) { return descriptorSetHandlers[set]; }
			const std::unordered_map<vk::DescriptorType, uint32_t>& getDescriptorTypeCount() const
				{ return descriptorTypeCount; }

			// Recreates the pipeline
			void recreate(vk::Extent2D extent, vk::RenderPass renderPass);

			// Binds the pipeline and per pipeline descriptors to the command buffer
			void bind(const vk::CommandBuffer& commandBuffer) const;
			// Binds per mesh descriptors
			void bindMeshDescriptors(const vk::CommandBuffer& commandBuffer, uint32_t index) const;

			// Allocates user descriptor set data in the descriptor pool
			void configureUserDescriptorData(const Device& mtdDevice, const DescriptorPool& pool);
			// Updates the user descriptor data for the specified binding
			void updateDescriptorData(uint32_t binding, const void* data) const;

		private:
			// Vulkan graphics pipeline
			vk::Pipeline pipeline;
			// Pipeline layout
			vk::PipelineLayout pipelineLayout;

			// Pipeline specific configurations
			GraphicsPipelineInfo info;

			// Shader modules used in the pipeline
			std::vector<ShaderModule> shaders;
			// Descriptor sets and their layouts
			std::vector<DescriptorSetHandler> descriptorSetHandlers;
			// Required descriptor count for each descriptor type of the current pipeline
			std::unordered_map<vk::DescriptorType, uint32_t> descriptorTypeCount;

			// Vulkan device reference
			const vk::Device& device;

			// Loads the pipeline shader modules
			void loadShaderModules();

			// Creates the layout for the pipeline
			void createPipelineLayout(const vk::DescriptorSetLayout& globalDescriptorSetLayout);
			// Creates the graphics pipeline
			void createPipeline(vk::Extent2D extent, vk::RenderPass renderPass);

			// Configures the descriptor set handlers to be used
			void createDescriptorSetLayouts();

			// Sets the input assembly create info
			void setInputAssembly(vk::PipelineInputAssemblyStateCreateInfo& inputAssemblyInfo) const;
			// Sets the viewport create info
			void setViewport
			(
				vk::PipelineViewportStateCreateInfo& viewportInfo,
				vk::Viewport& viewport,
				vk::Rect2D& scissor,
				vk::Extent2D extent
			) const;
			// Sets the rasterization create info
			void setRasterizer(vk::PipelineRasterizationStateCreateInfo& rasterizationInfo) const;
			// Sets the multisample create info
			void setMultisampling(vk::PipelineMultisampleStateCreateInfo& multisampleInfo) const;
			// Sets the depth stencil create info
			void setDepthStencil(vk::PipelineDepthStencilStateCreateInfo& depthStencilInfo) const;
	};
}