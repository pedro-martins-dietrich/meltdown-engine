#pragma once

#include "../Device/Device.hpp"

namespace mtd
{
	class Gui;

	// Handles Vulkan commands to the GPU
	class CommandHandler
	{
		public:
			CommandHandler(const Device& device);
			~CommandHandler();

			CommandHandler(const CommandHandler&) = delete;
			CommandHandler operator=(const CommandHandler&) = delete;

			CommandHandler(CommandHandler&& otherCommandHandler) noexcept;

			// Getter
			const vk::CommandBuffer& getCommandBuffer() const { return mainCommandBuffer; }

			// Allocates a command buffer in the command pool
			void allocateCommandBuffer(vk::CommandBuffer& commandBuffer) const;

			// Creates a command buffer for submitting a command once
			vk::CommandBuffer beginSingleTimeCommand() const;
			// Submits and finalizes the single time command
			void endSingleTimeCommand(const vk::CommandBuffer& commandBuffer) const;

			// Begins main command buffer
			void beginCommand() const;
			// Ends main command buffer
			void endCommand() const;

			// Submits recorded draw command
			void submitDrawCommandBuffer(const SynchronizationBundle& syncBudle) const;

		private:
			// Command buffer allocator
			vk::CommandPool commandPool;
			// Main command buffer for the handler
			vk::CommandBuffer mainCommandBuffer;

			// Device reference
			const Device& device;
	};
}
