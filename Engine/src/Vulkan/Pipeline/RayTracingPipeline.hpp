#pragma once

#include "ShaderModule.hpp"
#include "../Descriptors/DescriptorPool.hpp"

namespace mtd
{
	// Vulkan pipeline for ray trace based rendering
	class RayTracingPipeline
	{
		public:
			RayTracingPipeline
			(
				const Device& mtdDevice,
				const RayTracingPipelineInfo& info,
				const vk::DescriptorSetLayout& globalDescriptorSetLayout,
				vk::Extent2D swapchainExtent
			);
			~RayTracingPipeline();

			RayTracingPipeline(const RayTracingPipeline&) = delete;
			RayTracingPipeline& operator=(const RayTracingPipeline&) = delete;

			RayTracingPipeline(RayTracingPipeline&& other) noexcept;

			// Getters
			const std::string& getName() const { return info.pipelineName; }
			const vk::PipelineLayout& getLayout() const { return pipelineLayout; }
			DescriptorSetHandler& getDescriptorSetHandler(uint32_t set) { return descriptorSetHandlers[set]; }
			const std::unordered_map<vk::DescriptorType, uint32_t>& getDescriptorTypeCount() const
				{ return descriptorTypeCount; }

			// Binds the pipeline and performs the ray tracing
			void traceRays
			(
				const vk::CommandBuffer& commandBuffer, const vk::detail::DispatchLoaderDynamic& dldi
			) const;

			// Configures the render target image descriptor
			void configurePipelineDescriptorSet();

			// Allocates user descriptor set data in the descriptor pool
			void configureUserDescriptorData(const Device& mtdDevice, const DescriptorPool& pool);
			// Updates the user descriptor data for the specified binding
			void updateDescriptorData(uint32_t binding, const void* data) const;

			// Configures the render target image as a descriptor for another descriptor set
			void shareRenderTargetImageDescriptor
			(
				DescriptorSetHandler& descriptorSetHandler, uint32_t binding
			) const;

			// Resizes the render target image if needed
			void resize(const Device& mtdDevice, vk::Extent2D swapchainExtent);

		private:
			// Vulkan ray tracing pipeline
			vk::Pipeline pipeline;
			// Vulkan pipeline layout
			vk::PipelineLayout pipelineLayout;

			// Ray tracing pipeline specific configurations
			RayTracingPipelineInfo info;

			// Ray tracing shaders
			std::vector<ShaderModule> shaders;
			// Descriptor sets and their layouts
			std::vector<DescriptorSetHandler> descriptorSetHandlers;
			// Required descriptor count for each descriptor type of the current pipeline
			std::unordered_map<vk::DescriptorType, uint32_t> descriptorTypeCount;

			// Target image resources
			vk::Image image;
			vk::DeviceMemory imageMemory;
			vk::ImageView imageView;
			vk::Sampler sampler;
			bool windowResolutionDependant;

			// Shader Binding Table (SBT) buffer and the memory regions
			GpuBuffer sbtBuffer;
			vk::StridedDeviceAddressRegionKHR rayGenRegionSBT;
			vk::StridedDeviceAddressRegionKHR missRegionSBT;
			vk::StridedDeviceAddressRegionKHR hitRegionSBT;
			vk::StridedDeviceAddressRegionKHR callableRegionSBT;

			// Vulkan device reference
			const vk::Device& device;

			// Loads the pipeline shader modules
			void loadShaderModules();

			// Configures the descriptor set handlers to be used
			void createDescriptorSetLayouts();

			// Creates the storage image for the ray trace rendering
			void createStorageImage(const Device& mtdDevice, vk::Extent2D swapchainExtent);

			// Creates the layout for the pipeline
			void createPipelineLayout(const vk::DescriptorSetLayout& globalDescriptorSetLayout);
			// Creates the Vulkan ray tracing pipeline
			void createRayTracingPipeline(const vk::detail::DispatchLoaderDynamic& dldi);

			// Creates the Shader Binding Table (SBT) and the regions
			void createShaderBindingTable(const Device& mtdDevice);

			// Defines the shader groups
			void defineShaderGroups(std::vector<vk::RayTracingShaderGroupCreateInfoKHR>& shaderGroupCreateInfos) const;

			// Creates sampler to define how the render target image should be sampled
			void createSampler();
	};
}
