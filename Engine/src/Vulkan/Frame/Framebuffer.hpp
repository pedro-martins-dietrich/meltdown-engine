#pragma once

#include <meltdown/structs.hpp>

#include "../Descriptors/DescriptorSetHandler.hpp"
#include "../../Utils/EngineStructs.hpp"

namespace mtd
{
	// Vulkan framebuffer handler
	class Framebuffer
	{
		public:
			Framebuffer(const Device& mtdDevice, const FramebufferInfo& fbInfo, vk::Extent2D swapchainExtent);
			~Framebuffer();

			Framebuffer(const Framebuffer&) = delete;
			Framebuffer& operator=(const Framebuffer&) = delete;

			Framebuffer(Framebuffer&& other) noexcept;

			// Getters
			vk::Framebuffer getFramebuffer() const { return framebuffer; }
			vk::RenderPass getRenderPass() const { return renderPass; }
			vk::Extent2D getExtent() const { return {info.width, info.height}; }

			// Configures the specified attachment to be used as a descriptor
			void configureAttachmentAsDescriptor
			(
				DescriptorSetHandler& descriptorSetHandler, uint32_t binding, uint32_t attachmentIndex
			) const;

			// Creates an image barrier to transition the attachment layout
			void transitionAttachmentLayout
			(
				bool toShaderReadOnly,
				uint32_t attachmentIndex,
				vk::ImageMemoryBarrier& barrier,
				const vk::CommandBuffer& commandBuffer
			) const;

			// Resizes the framebuffer resolution if needed
			void resize(const Device& mtdDevice, vk::Extent2D swapchainExtent);

		private:
			// Vulkan framebuffer object
			vk::Framebuffer framebuffer;
			// Vulkan render pass associated with the framebuffer
			vk::RenderPass renderPass;
			// Sampler shared across the framebuffer attachments
			vk::Sampler sampler;

			// Framebuffer attachments
			std::vector<AttachmentData> attachmentsData;
			// Image information to use attachment as a descriptor
			std::vector<vk::DescriptorImageInfo> descriptorInfos;

			// Framebuffer configuration data
			FramebufferInfo info;
			// Indicator that the window and framebuffer resolutions are associated
			bool windowResolutionDependant;

			// Vulkan device reference
			const vk::Device& device;

			// Creates the Vulkan render pass for the framebuffer
			void createRenderPass();

			// Creates the attachment objects
			void createAttachments(const Device& mtdDevice, vk::Extent2D swapchainExtent);
			// Creates the data for a single attachment
			void createAttachment
			(
				const Device& mtdDevice,
				AttachmentData& attachmentData,
				vk::ImageUsageFlags usage,
				vk::ImageAspectFlags aspect
			) const;

			// Creates the Vulkan framebuffer object
			void createFramebuffer();

			// Creates sampler to define how the attachments should be rendered
			void createSampler();
	};
}
