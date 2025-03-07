#pragma once

#include "Pipeline.hpp"
#include "../Image/Image.hpp"

namespace mtd
{
	// Vulkan pipeline for ray trace based rendering
	class RayTracingPipeline : public Pipeline<RayTracingPipelineInfo>
	{
		public:
			RayTracingPipeline
			(
				const Device& mtdDevice,
				const RayTracingPipelineInfo& info,
				const vk::DescriptorSetLayout& globalDescriptorSetLayout,
				vk::Extent2D swapchainExtent
			);
			~RayTracingPipeline() = default;

			RayTracingPipeline(RayTracingPipeline&& other) noexcept;

			// Binds the pipeline and performs the ray tracing
			void traceRays
			(
				const vk::CommandBuffer& commandBuffer, const vk::detail::DispatchLoaderDynamic& dldi
			) const;

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
			// Render storage image
			Image image;
			// Flag to recreate image on window resize
			bool windowResolutionDependant;

			// Shader Binding Table (SBT) buffer and the memory regions
			GpuBuffer sbtBuffer;
			vk::StridedDeviceAddressRegionKHR rayGenRegionSBT;
			vk::StridedDeviceAddressRegionKHR missRegionSBT;
			vk::StridedDeviceAddressRegionKHR hitRegionSBT;
			vk::StridedDeviceAddressRegionKHR callableRegionSBT;

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
	};
}
