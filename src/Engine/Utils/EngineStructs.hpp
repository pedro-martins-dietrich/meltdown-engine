#pragma once

#include <glm/glm.hpp>
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
	};

	// Vertex format
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
	};

	// Vertex array manager draw data
	struct MeshLumpData
	{
		const std::vector<uint32_t>& indexCounts;
		const std::vector<uint32_t>& instanceCounts;
		const std::vector<uint32_t>& indexOffsets;
		const vk::Buffer& vertexBuffer;
		const vk::Buffer& indexBuffer;
	};

	// Information required for drawing a frame
	struct DrawInfo
	{
		MeshLumpData meshLumpData;
		const vk::Pipeline& pipeline;
		const vk::PipelineLayout& pipelineLayout;
		const vk::RenderPass& renderPass;
		const vk::SwapchainKHR& swapchain;
		const vk::Extent2D& extent;
		const vk::Framebuffer* framebuffer;
		const SynchronizationBundle* syncBundle;
		uint32_t frameIndex;
		const CameraMatrices* cameraMatrices;
	};
}
