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
			vk::Format getColorFormat() const { return colorFormat; }
			const Frame& getFrame(uint32_t index) const { return frames[index]; }

			// Create framebuffers for each frame
			void createFramebuffers(const vk::RenderPass& renderPass);

		private:
			// Vulkan swapchain
			vk::SwapchainKHR swapchain;
			// Features supported by the current device
			SwapchainSupportedDetails supportedDetails;

			// Frames stored by the swapchain
			std::vector<Frame> frames;
			// Number of frames
			uint32_t frameCount;

			// Frame size
			vk::Extent2D extent;
			// Image color format
			vk::Format colorFormat;

			// Vulkan device reference
			const vk::Device& device;

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

			// Selects the image format to be used
			vk::SurfaceFormatKHR selectFormat
			(
				vk::Format desiredFormat, vk::ColorSpaceKHR desiredColorSpace
			) const;
			// Sets how many frames will be stored in the buffer
			void selectImageCount();
			// Sets the frame dimensions to be used in the swapchain
			void selectExtent(const FrameDimensions& frameDimensions);
			// Sets the present mode to be used
			vk::PresentModeKHR selectPresentMode(vk::PresentModeKHR desiredPresentMode) const;

			// Creates all the swapchain frames
			void setSwapchainFrames(const Device& device, const FrameDimensions& frameDimensions);

			// Destroys the swapchain
			void destroy();
	};
}
