#pragma once

#include "../Image/Image.hpp"
#include "../../Utils/EngineStructs.hpp"

namespace mtd
{
	// Defines the properties of each frame
	class Frame
	{
		public:
			Frame
			(
				const Device& mtdDevice,
				const FrameDimensions& frameDimensions,
				vk::Image image,
				vk::Format format,
				uint32_t frameIndex
			);
			~Frame();

			Frame(const Frame&) = delete;
			Frame& operator=(const Frame&) = delete;

			Frame(Frame&& other) noexcept;

			// Getters
			vk::Format getDepthFormat() const { return depthBuffer.getFormat(); }
			const vk::Fence& getInFlightFence() const { return synchronizationBundle.inFlightFence; }
			const vk::Semaphore& getImageAvailableSemaphore() const { return synchronizationBundle.imageAvailable; }
			const CommandHandler& getCommandHandler() const { return commandHandler; }

			// Adds frame data to the draw info
			void fetchFrameDrawData(DrawInfo& drawInfo) const;

			// Set up framebuffer
			void createFramebuffer(const vk::RenderPass& renderPass);

		private:
			// Frame storage
			vk::Framebuffer framebuffer;

			// Color buffer attachment data
			Image colorBuffer;
			// Depth buffer attachment data
			Image depthBuffer;

			// Frame index in the swapchain
			uint32_t frameIndex;
			// Frame dimensions
			FrameDimensions frameDimensions;

			// Vulkan command handler
			CommandHandler commandHandler;

			// Synchronization objects
			SynchronizationBundle synchronizationBundle;

			// Vulkan device reference
			const vk::Device& device;

			// Creates depth buffer data
			void createDepthResources(const Device& mtdDevice);

			// Selects an image format with the specified features
			vk::Format findSupportedFormat
			(
				const vk::PhysicalDevice& physicalDevice,
				const std::vector<vk::Format>& candidates,
				vk::ImageTiling tiling,
				vk::FormatFeatureFlags features
			) const;
	};
}
