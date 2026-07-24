#pragma once

#include "../Device/Device.hpp"
#include "../../Utils/EngineStructs.hpp"

namespace mtd
{
	// Handles Vulkan commands to the GPU
	class CommandHandler
	{
		public:
			CommandHandler(const Device& mtdDevice);
			~CommandHandler();

			CommandHandler(const CommandHandler&) = delete;
			CommandHandler operator=(const CommandHandler&) = delete;

			CommandHandler(CommandHandler&& otherCommandHandler) noexcept;

			// Fetches the main command buffer
			const vk::CommandBuffer& getCommandBuffer() const { return mainCommandBuffer; }

			// Allocates a command buffer in the command pool
			void allocateCommandBuffer(vk::CommandBuffer& commandBuffer) const;

			// Creates a command buffer for submitting a command once
			vk::CommandBuffer beginSingleTimeCommand() const;
			// Submits and finalizes the single time command
			void endSingleTimeCommand(const vk::CommandBuffer& commandBuffer) const;

			// Begins the main command buffer
			void beginCommand() const;
			// Ends the main command buffer
			void endCommand() const;

			// Submits recorded draw command
			void submitDrawCommandBuffer(const SynchronizationBundle& syncBundle) const;

		private:
			// Command buffer allocator
			vk::CommandPool commandPool;
			// Main command buffer for the handler
			vk::CommandBuffer mainCommandBuffer;

			// Device reference
			const Device& mtdDevice;
	};
}
