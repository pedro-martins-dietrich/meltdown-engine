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
				const Device& mtdDevice, UIntVec2 frameDimensions,
				vk::Image colorBufferImage, vk::Format colorBufferFormat
			);
			~Frame();

			Frame(const Frame&) = delete;
			Frame& operator=(const Frame&) = delete;

			Frame(Frame&& other) noexcept;

			// Getters
			UIntVec2 getDimensions() const { return dimensions; }
			vk::Framebuffer getFramebuffer() const { return framebuffer; }
			vk::Image getColorBufferImage() const { return colorBuffer; }
			vk::Format getDepthFormat() const { return depthBuffer.getFormat(); }
			const SynchronizationBundle& getSyncBundle() const { return synchronizationBundle; }
			const CommandHandler& getCommandHandler() const { return commandHandler; }

			// Sets up framebuffer
			void createFramebuffer(const vk::RenderPass& renderPass);

		private:
			// Frame storage
			vk::Framebuffer framebuffer;
			// Frame dimensions
			UIntVec2 dimensions;

			// Color buffer Vulkan image
			vk::Image colorBuffer;
			// Color buffer view
			vk::ImageView colorBufferView;

			// Depth buffer attachment data
			Image depthBuffer;

			// Command handler for frame specific operations
			CommandHandler commandHandler;

			// Synchronization objects
			SynchronizationBundle synchronizationBundle;

			// Vulkan device reference
			const vk::Device& device;

			// Creates the image view for the color buffer
			void createColorBufferView(vk::Format colorBufferFormat);
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
