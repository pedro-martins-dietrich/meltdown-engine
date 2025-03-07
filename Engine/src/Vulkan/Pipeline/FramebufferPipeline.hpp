#pragma once

#include "RayTracingPipeline.hpp"
#include "../Frame/Framebuffer.hpp"

namespace mtd
{
	// Vulkan graphics pipeline to handle framebuffers
	class FramebufferPipeline : public Pipeline<FramebufferPipelineInfo>
	{
		public:
			FramebufferPipeline
			(
				const vk::Device& device,
				const FramebufferPipelineInfo& info,
				const vk::DescriptorSetLayout& globalDescriptorSetLayout,
				vk::Extent2D extent,
				vk::RenderPass renderPass
			);
			~FramebufferPipeline() = default;

			FramebufferPipeline(FramebufferPipeline&& other) noexcept;

			// Getters
			int32_t getTargetFramebufferIndex() const { return info.targetFramebufferIndex; }
			const std::vector<uint32_t>& getPipelineIndices() const { return info.dependencies; }
			const std::vector<AttachmentIdentifier>& getAttachmentIdentifiers() const { return info.inputAttachments; }
			uint32_t getImageDescriptorsCount() const
				{ return static_cast<uint32_t>(info.inputAttachments.size() + info.rayTracingStorageImages.size()); }

			// Recreates the framebuffer pipeline
			void recreate(vk::Extent2D extent, vk::RenderPass renderPass);

			// Binds the pipeline and per pipeline descriptors to the command buffer
			void bind(const vk::CommandBuffer& commandBuffer) const;

			// Updates all the input images descriptors
			void updateInputImagesDescriptors
			(
				const std::vector<Framebuffer>& framebuffers,
				const std::vector<RayTracingPipeline>& rayTracingPipelines
			);

		private:
			// Loads the pipeline shader modules
			void loadShaderModules();

			// Creates the layout for the framebuffer pipeline
			void createPipelineLayout(const vk::DescriptorSetLayout& globalDescriptorSetLayout);
			// Creates the graphics pipeline
			void createPipeline(vk::Extent2D extent, vk::RenderPass renderPass);

			// Configures the descriptor set handlers to be used
			void createDescriptorSetLayouts();

			// Sets the vertex input create info
			void setVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo) const;
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
