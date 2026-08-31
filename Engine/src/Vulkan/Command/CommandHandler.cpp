#include <pch.hpp>
#include "CommandHandler.hpp"

#include "../../Utils/Logger.hpp"

mtd::CommandHandler::CommandHandler(const Device& mtdDevice) : mtdDevice{mtdDevice}
{
	vk::CommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	commandPoolCreateInfo.queueFamilyIndex = mtdDevice.getQueueFamilies().getGraphicsFamilyIndex();

	vk::Result result = mtdDevice.getDevice().createCommandPool(&commandPoolCreateInfo, nullptr, &commandPool);
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
	mtdDevice.getDevice().destroyCommandPool(commandPool);
}

mtd::CommandHandler::CommandHandler(CommandHandler&& other) noexcept
	: mtdDevice{other.mtdDevice},
	commandPool{std::move(other.commandPool)},
	mainCommandBuffer{std::move(other.mainCommandBuffer)}
{
	other.commandPool = nullptr;
	other.mainCommandBuffer = nullptr;
}

void mtd::CommandHandler::allocateCommandBuffer(vk::CommandBuffer& commandBuffer) const
{
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	commandBufferAllocateInfo.commandBufferCount = 1U;

	vk::Result result = mtdDevice.getDevice().allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to allocate command buffer. Vulkan result: %d", result);
}

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

void mtd::CommandHandler::endSingleTimeCommand
(
	const vk::CommandBuffer& commandBuffer,
	vk::Semaphore waitSemaphore,
	vk::Semaphore signalSemaphore,
	vk::Fence fence
) const
{
	commandBuffer.end();

	vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo{};
	submitInfo.waitSemaphoreCount = waitSemaphore ? 1U : 0U;
	submitInfo.pWaitSemaphores = waitSemaphore ? &waitSemaphore : nullptr;
	submitInfo.pWaitDstStageMask = waitSemaphore ? &waitStage : nullptr;
	submitInfo.commandBufferCount = 1U;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = signalSemaphore ? 1U : 0U;
	submitInfo.pSignalSemaphores = signalSemaphore ? &signalSemaphore : nullptr;

	vk::Result result = mtdDevice.getGraphicsQueue().submit(1U, &submitInfo, fence);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to submit command buffer. Vulkan result: %d", result);

	mtdDevice.getGraphicsQueue().waitIdle();

	mtdDevice.getDevice().freeCommandBuffers(commandPool, 1U, &commandBuffer);
}

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

void mtd::CommandHandler::endCommand() const
{
	mainCommandBuffer.end();
}

void mtd::CommandHandler::submitDrawCommandBuffer(const SynchronizationBundle& syncBundle) const
{
	vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo{};
	submitInfo.waitSemaphoreCount = 1U;
	submitInfo.pWaitSemaphores = &(syncBundle.imageAvailable);
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1U;
	submitInfo.pCommandBuffers = &mainCommandBuffer;
	submitInfo.signalSemaphoreCount = 1U;
	submitInfo.pSignalSemaphores = &(syncBundle.renderFinished);

	vk::Result result = mtdDevice.getGraphicsQueue().submit(1U, &submitInfo, syncBundle.inFlightFence);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to submit draw command to the GPU. Vulkan result: %d", result);
}
