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

		private:
			// Vulkan swapchain
			vk::SwapchainKHR swapchain;
			// Features supported by the current device
			SwapchainSupportedDetails supportedDetails;

			// Frames stored by the swapchain
			std::vector<Frame> frames;
			// Number of frames
			uint32_t frameCount;

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
			uint32_t selectImageCount(uint32_t desiredImageCount) const;
			// Sets the frame dimensions to be used in the swapchain
			vk::Extent2D selectExtent(const FrameDimensions& frameDimensions) const;
			// Sets the present mode to be used
			vk::PresentModeKHR selectPresentMode(vk::PresentModeKHR desiredPresentMode) const;

			// Creates all the swapchain frames
			void setSwapchainFrames
			(
				const FrameDimensions& frameDimensions, vk::Format format
			);

			// Destroys the swapchain
			void destroy();
	};
}
