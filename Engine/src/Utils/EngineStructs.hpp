#pragma once

#include <unordered_map>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace mtd
{
	// Information about the size and position of a window
	struct WindowInfo
	{
		int width;
		int height;
		int posX;
		int posY;
	};

	// Width and height of the frame
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

	// Customizable swapchain settings
	struct SwapchainSettings
	{
		uint32_t frameCount;
		vk::Format colorFormat;
		vk::ColorSpaceKHR colorSpace;
		vk::CompositeAlphaFlagBitsKHR compositeAlpha;
		vk::PresentModeKHR presentMode;
	};

	// Customizable pipeline settings
	struct PipelineSettings
	{
		vk::PrimitiveTopology inputAssemblyPrimitiveTopology;
		vk::PolygonMode rasterizationPolygonMode;
		vk::CullModeFlags rasterizationCullMode;
		vk::FrontFace rasterizationFrontFace;
	};

	// Frame synchronization bundle
	struct SynchronizationBundle
	{
		// Blocks CPU execution until frame is ready
		vk::Fence inFlightFence;
		// Signals the image has been aquired
		vk::Semaphore imageAvailable;
		// Signals the image has been rendered in the frame
		vk::Semaphore renderFinished;
	};

	// Camera data to send to the shaders
	struct CameraMatrices
	{
		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 projectionView;
	};

	// Vertex format
	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 textureCoordinates;
		glm::vec3 normal;
		glm::vec3 color;
	};

	// Information required for drawing a frame
	struct DrawInfo
	{
		const vk::RenderPass& renderPass;
		const vk::Extent2D& extent;
		const vk::DescriptorSet& globalDescriptorSet;
		const vk::Framebuffer* framebuffer;
		const SynchronizationBundle* syncBundle;
	};
}
