#pragma once

#include "../Device/Device.hpp"
#include "Frame.hpp"

namespace mtd
{
	// Handles frame swapping
	class Swapchain
	{
		public:
			Swapchain
			(
				const Device& device,
				const FrameDimensions& frameDimensions,
				const vk::SurfaceKHR& surface
			);
			~Swapchain();

			Swapchain(const Swapchain&) = delete;
			Swapchain& operator=(const Swapchain&) = delete;

			// Getters
			const vk::SwapchainKHR& getSwapchain() const { return swapchain; }
			const vk::Extent2D& getExtent() const { return extent; }
			const vk::RenderPass& getRenderPass() const { return renderPass; }
			const Frame& getFrame(uint32_t index) const { return frames[index]; }
			SwapchainSettings& getSettings() { return settings; }

			// Recreates swapchain to handle resizes
			void recreate
			(
				const Device& device,
				const FrameDimensions& frameDimensions,
				const vk::SurfaceKHR& surface
			);

		private:
			// Vulkan swapchain
			vk::SwapchainKHR swapchain;
			// Features supported by the current device
			SwapchainSupportedDetails supportedDetails;
			// Vulkan render pass shared among the pipelines
			vk::RenderPass renderPass;

			// Frames stored by the swapchain
			std::vector<Frame> frames;

			// Frame size
			vk::Extent2D extent;

			// Customizable swapchain settings
			SwapchainSettings settings;

			// Vulkan device reference
			const vk::Device& device;

			// Sets up default swapchain settings
			void configureDefaultSettings();

			// Retrieves swapchain features supported by the physical device
			void getSupportedDetails
			(
				const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface
			);
			// Creates the swapchain
			void createSwapchain
			(
				const Device& device,
				const FrameDimensions& frameDimensions,
				const vk::SurfaceKHR& surface
			);
			// Creates render pass
			void createRenderPass();
			// Create framebuffers for each frame
			void createFramebuffers();

			// Ensures the swapchain uses a valid surface format
			void checkSurfaceFormat();
			// Ensures a valid amount of frames to be stored in the buffer
			void checkImageCount();
			// Sets the frame dimensions to be used in the swapchain
			void selectExtent(const FrameDimensions& frameDimensions);
			// Ensures the present mode to be used is valid
			void checkPresentMode();

			// Creates all the swapchain frames
			void setSwapchainFrames(const Device& device, const FrameDimensions& frameDimensions);

			// Destroys the swapchain
			void destroy();
	};
}
