#pragma once

#include "Pipeline.hpp"

namespace mtd
{
	// Vulkan graphics (rasterization) pipeline handler
	class GraphicsPipeline : public Pipeline<GraphicsPipelineInfo>
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
			~GraphicsPipeline() = default;

			GraphicsPipeline(GraphicsPipeline&& other) noexcept;

			// Getters
			int32_t getTargetFramebuffer() const { return info.targetFramebufferIndex; }
			MeshType getAssociatedMeshType() const { return info.associatedMeshType; }

			// Recreates the pipeline
			void recreate(vk::Extent2D extent, vk::RenderPass renderPass);

			// Binds the pipeline and per pipeline descriptors to the command buffer
			void bind(const vk::CommandBuffer& commandBuffer) const;
			// Binds per mesh descriptors
			void bindMeshDescriptors(const vk::CommandBuffer& commandBuffer, uint32_t index) const;

		private:
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