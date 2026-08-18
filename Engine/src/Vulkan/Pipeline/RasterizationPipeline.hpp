#pragma once

#include "Pipeline.hpp"
#include "../Descriptors/DescriptorManager.hpp"

namespace mtd
{
	// Vulkan rasterization pipeline handler
	class RasterizationPipeline : public Pipeline<RasterizationPipelineInfo>
	{
		public:
			RasterizationPipeline
			(
				const vk::Device& device,
				const DescriptorManager& descriptorManager,
				const RasterizationPipelineInfo& info,
				vk::Extent2D extent,
				vk::RenderPass renderPass
			);
			~RasterizationPipeline() = default;

			RasterizationPipeline(RasterizationPipeline&& other) noexcept;

			// Getters
			int32_t getTargetFramebuffer() const { return info.targetFramebufferIndex; }
			MeshType getAssociatedMeshType() const { return info.associatedMeshType; }

			// Recreates the pipeline
			void recreate(vk::Extent2D extent, vk::RenderPass renderPass);

			// Binds the pipeline and per pipeline descriptors to the command buffer
			void bind(vk::CommandBuffer commandBuffer) const;

			// Pushes a constant to the pipeline draw call
			void pushConstant(vk::CommandBuffer commandBuffer, const uint32_t& constantData) const;

		private:
			// Loads the pipeline shader modules
			void loadShaderModules();

			// Creates the layout for the pipeline
			void createPipelineLayout();
			// Creates the rasterization pipeline
			void createPipeline(vk::Extent2D extent, vk::RenderPass renderPass);

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