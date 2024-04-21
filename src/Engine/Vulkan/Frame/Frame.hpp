#pragma once

#include <vulkan/vulkan.hpp>

#include "../../Utils/EngineStructs.hpp"
#include "../Command/CommandHandler.hpp"

namespace mtd
{
	// Defines the properties of each frame
	class Frame
	{
		public:
			Frame
			(
				const Device& device,
				const FrameDimensions& frameDimensions,
				vk::Image image,
				vk::Format format,
				uint32_t frameIndex
			);
			~Frame();

			Frame(const Frame&) = delete;
			Frame& operator=(const Frame&) = delete;

			Frame(Frame&& otherFrame) noexcept;

			// Getters
			const vk::Fence& getInFlightFence() const
				{ return synchronizationBundle.inFlightFence; }
			const vk::Semaphore& getImageAvailableSemaphore() const
				{ return synchronizationBundle.imageAvailable; }

			// Set up framebuffer
			void createFramebuffer(const vk::RenderPass& renderPass);

			// Draws frame to screen
			void drawFrame(DrawInfo& drawInfo) const;

		private:
			// Image data
			vk::Image image;
			// Image desctiption
			vk::ImageView imageView;
			// Frame storage
			vk::Framebuffer framebuffer;

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

			// Create frame image view
			void createImageView(vk::Format format);
	};
}
