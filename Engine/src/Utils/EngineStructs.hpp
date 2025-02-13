#pragma once

#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>

#include <meltdown/enums.hpp>
#include <meltdown/math.hpp>

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

	// Data associated with a Vulkan attachment
	struct AttachmentData
	{
		vk::Image image;
		vk::ImageView imageView;
		vk::DeviceMemory imageMemory;
		vk::Format format = vk::Format::eUndefined;
	};

	// Information about the pipelines that targets a framebuffer
	struct RenderPassInfo
	{
		int32_t targetFramebufferIndex;
		std::optional<uint32_t> framebufferPipelineIndex = std::nullopt;
		std::vector<uint32_t> pipelineIndices = {};
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
		Mat4x4 projection;
		Mat4x4 view;
		Mat4x4 projectionView;
	};

	// Vertex format
	struct Vertex
	{
		Vec3 position;
		Vec2 textureCoordinates;
		Vec3 normal;
	};

	// Information about the attributes for a specific material type
	struct MaterialInfo
	{
		std::vector<MaterialFloatDataType> floatDataTypes;
		std::vector<MaterialTextureType> textureTypes;
	};

	// Information for submesh rendering
	struct SubmeshData
	{
		uint32_t indexOffset;
		uint32_t materialID;
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
