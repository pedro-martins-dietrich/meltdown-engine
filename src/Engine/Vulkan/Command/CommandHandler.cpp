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

	vk::Result result =
		device.getDevice().allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to allocate command buffer. Vulkan result: %d", result);
}

// Draws frame
void mtd::CommandHandler::draw(const DrawInfo& drawInfo) const
{
	recordDrawCommand(drawInfo);
	submitCommandBuffer(*drawInfo.syncBundle);
	presentFrame(drawInfo);
}

// Begin command buffer
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

// End command buffer
void mtd::CommandHandler::endCommand() const
{
	mainCommandBuffer.end();
}

// Records draw command to the command buffer
void mtd::CommandHandler::recordDrawCommand(const DrawInfo& drawInfo) const
{
	beginCommand();

	vk::Rect2D renderArea{};
	renderArea.offset = vk::Offset2D{0, 0};
	renderArea.extent = *drawInfo.extent;

	std::vector<vk::ClearValue> clearValues;
	clearValues.push_back(vk::ClearColorValue{0.3f, 0.6f, 1.0f, 1.0f});

	vk::RenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.renderPass = *drawInfo.renderPass;
	renderPassBeginInfo.framebuffer = *drawInfo.framebuffer;
	renderPassBeginInfo.renderArea = renderArea;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	mainCommandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
	mainCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *drawInfo.pipeline);
	mainCommandBuffer.pushConstants
	(
		*drawInfo.pipelineLayout,
		vk::ShaderStageFlagBits::eVertex,
		0,
		sizeof(CameraMatrices),
		drawInfo.cameraMatrices
	);
	mainCommandBuffer.draw(3, 1, 0, 0);
	mainCommandBuffer.endRenderPass();

	endCommand();
}

// Submits recorded draw command
void mtd::CommandHandler::submitCommandBuffer(const SynchronizationBundle& syncBundle) const
{
	vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo{};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &syncBundle.imageAvailable;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mainCommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &syncBundle.renderFinished;

	vk::Result result =
		device.getGraphicsQueue().submit(1, &submitInfo, syncBundle.inFlightFence);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to submit draw command to the GPU. Vulkan result: %d", result);
}

// Presents frame to screen when ready
void mtd::CommandHandler::presentFrame(const DrawInfo& drawInfo) const
{
	vk::PresentInfoKHR presentInfo{};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &(drawInfo.syncBundle->renderFinished);
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = drawInfo.swapchain;
	presentInfo.pImageIndices = &drawInfo.frameIndex;
	presentInfo.pResults = nullptr;

	vk::Result result = device.getPresentQueue().presentKHR(&presentInfo);
	if(result != vk::Result::eSuccess && result != vk::Result::eErrorOutOfDateKHR)
		LOG_ERROR("Failed to present frame to screen. Vulkan result: %d", result);
}
