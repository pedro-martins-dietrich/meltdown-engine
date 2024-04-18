#pragma once

#include <vulkan/vulkan.hpp>

namespace mtd
{
	// Width and height of the window/frame
	struct FrameDimensions
	{
		uint32_t width;
		uint32_t height;
	};

	// Swapchain features supported by the device
	struct SwapchainSupportedDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};
}
