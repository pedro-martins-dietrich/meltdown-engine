#include "CommandHandler.hpp"

#include "../../Utils/Logger.hpp"

mtd::CommandHandler::CommandHandler(const Device& device) : device{device.getDevice()}
{
	vk::CommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.flags = vk::CommandPoolCreateFlags() |
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	commandPoolCreateInfo.queueFamilyIndex = device.getQueueFamilies().getGraphicsFamilyIndex();

	vk::Result result =
		this->device.createCommandPool(&commandPoolCreateInfo, nullptr, &commandPool);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create command pool. Vulkan result: %d", result);
		return;
	}
	LOG_VERBOSE("Created command pool.");

	allocateCommandBuffer(mainCommandBuffer);
}

mtd::CommandHandler::~CommandHandler()
{
	device.destroyCommandPool(commandPool);
}

mtd::CommandHandler::CommandHandler(CommandHandler&& otherCommandHandler) noexcept
	: device{otherCommandHandler.device},
	commandPool{std::move(otherCommandHandler.commandPool)},
	mainCommandBuffer{std::move(otherCommandHandler.mainCommandBuffer)}
{
	otherCommandHandler.commandPool = nullptr;
	otherCommandHandler.mainCommandBuffer = nullptr;
}

// Allocates a command buffer in the command pool
void mtd::CommandHandler::allocateCommandBuffer(vk::CommandBuffer& commandBuffer) const
{
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	commandBufferAllocateInfo.commandBufferCount = 1;

	vk::Result result = device.allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to allocate command buffer. Vulkan result: %d", result);
}
