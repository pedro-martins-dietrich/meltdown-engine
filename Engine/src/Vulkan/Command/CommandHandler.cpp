#include <pch.hpp>
#include "CommandHandler.hpp"

#include "../../Utils/Logger.hpp"

mtd::CommandHandler::CommandHandler(const Device& device) : device{device}
{
	vk::CommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.flags = vk::CommandPoolCreateFlags() |
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	commandPoolCreateInfo.queueFamilyIndex = device.getQueueFamilies().getGraphicsFamilyIndex();

	vk::Result result =
		device.getDevice().createCommandPool(&commandPoolCreateInfo, nullptr, &commandPool);
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
	device.getDevice().destroyCommandPool(commandPool);
}

mtd::CommandHandler::CommandHandler(CommandHandler&& other) noexcept
	: device{other.device},
	commandPool{std::move(other.commandPool)},
	mainCommandBuffer{std::move(other.mainCommandBuffer)}
{
	other.commandPool = nullptr;
	other.mainCommandBuffer = nullptr;
}

// Allocates a command buffer in the command pool
void mtd::CommandHandler::allocateCommandBuffer(vk::CommandBuffer& commandBuffer) const
{
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	commandBufferAllocateInfo.commandBufferCount = 1;

	vk::Result result =
		device.getDevice().allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to allocate command buffer. Vulkan result: %d", result);
}

// Creates a command buffer for submitting a command once
vk::CommandBuffer mtd::CommandHandler::beginSingleTimeCommand() const
{
	vk::CommandBuffer commandBuffer;
	allocateCommandBuffer(commandBuffer);

	vk::CommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	vk::Result result = commandBuffer.begin(&commandBufferBeginInfo);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to begin command buffer. Vulkan result: %d", result);

	return commandBuffer;
}

// Submits the single time command and frees the command buffer
void mtd::CommandHandler::endSingleTimeCommand(const vk::CommandBuffer& commandBuffer) const
{
	commandBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	(void) device.getGraphicsQueue().submit(1, &submitInfo, nullptr);
	device.getGraphicsQueue().waitIdle();

	device.getDevice().freeCommandBuffers(commandPool, 1, &commandBuffer);
}

// Begins main command buffer
void mtd::CommandHandler::beginCommand() const
{
	mainCommandBuffer.reset();

	vk::CommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	vk::Result result = mainCommandBuffer.begin(&commandBufferBeginInfo);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to begin command buffer. Vulkan result: %d", result);
}

// Ends main command buffer
void mtd::CommandHandler::endCommand() const
{
	mainCommandBuffer.end();
}

// Submits recorded draw command
void mtd::CommandHandler::submitDrawCommandBuffer(const SynchronizationBundle& syncBundle) const
{
	vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo{};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &(syncBundle.imageAvailable);
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mainCommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &(syncBundle.renderFinished);

	vk::Result result =
		device.getGraphicsQueue().submit(1, &submitInfo, syncBundle.inFlightFence);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to submit draw command to the GPU. Vulkan result: %d", result);
}
