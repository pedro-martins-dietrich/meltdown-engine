#pragma once

#include <meltdown/structs.hpp>

#include "ShaderModule.hpp"
#include "../Frame/Swapchain.hpp"
#include "../Descriptors/DescriptorPool.hpp"

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
				DescriptorSetHandler* globalDescriptorSet,
				const PipelineInfo& info
			);
			~Pipeline();

			Pipeline(const Pipeline&) = delete;
			Pipeline& operator=(const Pipeline&) = delete;

			Pipeline(Pipeline&& other) noexcept;

			// Getters
			const vk::Pipeline& getPipeline() const { return pipeline; }
			const vk::PipelineLayout& getLayout() const { return pipelineLayout; }
			const std::string& getName() const { return info.pipelineName; }
			MeshType getAssociatedMeshType() const { return info.associatedMeshType; }
			DescriptorSetHandler& getDescriptorSetHandler(uint32_t set) { return descriptorSetHandlers[set]; }
			const std::unordered_map<vk::DescriptorType, uint32_t>& getDescriptorTypeCount() const
				{ return descriptorTypeCount; }

			// Recreates the pipeline
			void recreate(Swapchain& swapchain, DescriptorSetHandler* globalDescriptorSet);

			// Allocates user descriptor set data in the descriptor pool
			void configureUserDescriptorData(const Device& mtdDevice, const DescriptorPool& pool);
			// Updates the user descriptor data for the specified binding
			void updateDescriptorData(uint32_t binding, const void* data) const;

			// Binds the pipeline to the command buffer
			void bind(const vk::CommandBuffer& commandBuffer) const;
			// Binds per pipeline descriptors
			void bindPipelineDescriptors(const vk::CommandBuffer& commandBuffer) const;
			// Binds per mesh descriptors
			void bindMeshDescriptors(const vk::CommandBuffer& commandBuffer, uint32_t index) const;

		private:
			// Vulkan graphics pipeline
			vk::Pipeline pipeline;
			// Pipeline layout
			vk::PipelineLayout pipelineLayout;

			// Pipeline specific configurations
			PipelineInfo info;

			// Shader modules used in the pipeline
			std::vector<ShaderModule> shaders;
			// Descriptor sets and their layouts
			std::vector<DescriptorSetHandler> descriptorSetHandlers;
			// Required descriptor count for each descriptor type of the current pipeline
			std::unordered_map<vk::DescriptorType, uint32_t> descriptorTypeCount;

			// Vulkan device reference
			const vk::Device& device;

			// Loads the pipeline shader modules
			void loadShaderModules(const char* vertexShaderPath, const char* fragmentShaderPath);

			// Creates the graphics pipeline
			void createPipeline(Swapchain& swapchain, DescriptorSetHandler* globalDescriptorSet);

			// Configures the descriptor set handlers to be used
			void createDescriptorSetLayouts();

			// Sets create infos for pipeline creation
			void setInputAssembly(vk::PipelineInputAssemblyStateCreateInfo& inputAssemblyInfo) const;
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