#pragma once

#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>

#include <meltdown/enums.hpp>
#include <meltdown/math.hpp>

namespace mtd
{
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

	// Common header format for asset files
	struct AssetHeader
	{
		uint64_t magic;
		uint64_t version;
	};

	// Common block header for asset files
	struct AssetBlockHeader
	{
		uint64_t blockID;
		uint64_t blockSize;
	};

	// Scene instance data
	struct SceneInstance
	{
		Mat4x4 transform;
		uint32_t pipelineID;
		uint32_t meshID;
		uint32_t materialSetID;
		bool visible = true;
	};

	// Data used by the GPU for each render object
	struct RenderObject
	{
		Mat4x4 transform;
		uint32_t materialSetID;
		uint32_t submeshOffset;
		uint32_t submeshCount;
		uint32_t vertexOffset;
		Vec3 centerAABB = Vec3{0.0f};
		float padding1 = 0.0f;
		Vec3 extentAABB = Vec3{0.0f};
		float padding2 = 0.0f;
	};

	// Drawing data for each draw batch call
	struct DrawBatch
	{
		uint32_t pipelineID;
		uint32_t meshID;
		uint32_t firstInstance;
		uint32_t instanceCount;
	};

	// Vertex format
	struct Vertex
	{
		Vec3 position;
		Vec2 textureCoordinates;
		Vec3 normal;
	};

	// Texture data for loading from the disk to the GPU
	struct TextureInfo
	{
		std::string texturePath;
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
		uint32_t indexCount;
		uint32_t materialSlot;
		uint32_t padding0;
		Vec3 centerAABB = Vec3{0.0f};
		float padding1;
		Vec3 extentAABB = Vec3{0.0f};
		float padding2;
	};

	// Information for the mesh rendering
	struct MeshData
	{
		uint32_t vertexStride;
		uint32_t vertexOffset;
		uint32_t indexOffset;
		uint32_t materialSlotCount;
		Vec3 centerAABB = Vec3{0.0f};
		Vec3 extentAABB = Vec3{0.0f};
		std::vector<SubmeshData> submeshes;
		uint32_t submeshOffset;
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

	// Compute shader push constant struct
	struct ComputeShaderPushConstantData
	{
		UIntVec2 imagesSize = UIntVec2{0U, 0U};
		uint32_t iterationCounter = 0U;
	};

	// Ray tracing push constant struct
	struct RayTracingRenderData
	{
		uint32_t samplesPerPixel = 1U;
		uint32_t maxRecursionDepth = 4U;
		uint32_t maxScatterRayCount = 4U;
		uint32_t accumulatedFrames = 0U;
		uint32_t randomSeed = 0U;
	};
}
