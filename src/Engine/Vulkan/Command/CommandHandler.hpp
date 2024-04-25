#pragma once

#include "../Device/Device.hpp"

namespace mtd
{
	// Handles Vulkan commands to the GPU
	class CommandHandler
	{
		public:
			CommandHandler(const Device& device);
			~CommandHandler();

			CommandHandler(const CommandHandler&) = delete;
			CommandHandler operator=(const CommandHandler&) = delete;

			CommandHandler(CommandHandler&& otherCommandHandler) noexcept;

			// Allocates a command buffer in the command pool
			void allocateCommandBuffer(vk::CommandBuffer& commandBuffer) const;

			// Creates a command buffer for submitting a command once
			vk::CommandBuffer beginSingleTimeCommand() const;
			// Submits and finalizes the single time command
			void endSingleTimeCommand(const vk::CommandBuffer& commandBuffer) const;

			// Draws frame
			void draw(const DrawInfo& drawInfo) const;

		private:
			// Command buffer allocator
			vk::CommandPool commandPool;
			// Main command buffer for the handler
			vk::CommandBuffer mainCommandBuffer;

			// Device reference
			const Device& device;

			// Begin command buffer
			void beginCommand() const;
			// End command buffer
			void endCommand() const;

			// Records draw command to the command buffer
			void recordDrawCommand(const DrawInfo& drawInfo) const;
			// Submits recorded draw command
			void submitCommandBuffer(const SynchronizationBundle& syncBudle) const;
			// Presents frame to screen when ready
			void presentFrame(const DrawInfo& drawInfo) const;
	};
}
