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

		private:
			// Command buffer allocator
			vk::CommandPool commandPool;
			// Main command buffer for the handler
			vk::CommandBuffer mainCommandBuffer;

			// Vulkan device reference
			const vk::Device& device;
	};
}
