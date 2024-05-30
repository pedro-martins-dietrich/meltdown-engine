#include "Renderer.hpp"

#include "../../Utils/Logger.hpp"

mtd::Renderer::Renderer()
	: currentFrameIndex{0}
{
}

// Renders frame to screen
void mtd::Renderer::render
(
	const Device& mtdDevice,
	const Swapchain& swapchain,
	const Gui& gui,
	DrawInfo& drawInfo,
	bool& shouldUpdateEngine
)
{
	const vk::Device& device = mtdDevice.getDevice();
	const Frame& frame = swapchain.getFrame(currentFrameIndex);
	const vk::Fence& inFlightFence = frame.getInFlightFence();

	(void) device.waitForFences(1, &inFlightFence, vk::True, UINT64_MAX);
	(void) device.resetFences(1, &inFlightFence);

	vk::Result result = device.acquireNextImageKHR
	(
		swapchain.getSwapchain(),
		UINT64_MAX,
		frame.getImageAvailableSemaphore(),
		nullptr,
		&currentFrameIndex
	);
	if(result != vk::Result::eSuccess)
	{
		if(result == vk::Result::eErrorOutOfDateKHR ||
			result == vk::Result::eErrorIncompatibleDisplayKHR)
		{
			currentFrameIndex = 0;
			shouldUpdateEngine = true;
		}
		else
		{
			LOG_ERROR("Failed to acquire swapchain image. Vulkan result: %d", result);
		}
		return;
	}

	swapchain.getFrame(currentFrameIndex).fetchFrameDrawData(drawInfo);
	const CommandHandler& commandHandler =
		swapchain.getFrame(currentFrameIndex).getCommandHandler();

	recordDrawCommand(commandHandler, drawInfo, gui);
	commandHandler.submitDrawCommandBuffer(*(drawInfo.syncBundle));
	presentFrame
	(
		swapchain.getSwapchain(),
		mtdDevice.getPresentQueue(),
		drawInfo.syncBundle->renderFinished
	);

	currentFrameIndex =
		shouldUpdateEngine ? 0 : (currentFrameIndex + 1) % swapchain.getFrameCount();
}

// Records draw command to the command buffer
void mtd::Renderer::recordDrawCommand
(
	const CommandHandler& commandHandler, const DrawInfo& drawInfo, const Gui& gui
) const
{
	commandHandler.beginCommand();

	vk::Rect2D renderArea{};
	renderArea.offset = vk::Offset2D{0, 0};
	renderArea.extent = drawInfo.extent;

	std::vector<vk::ClearValue> clearValues;
	clearValues.push_back(vk::ClearColorValue{0.3f, 0.6f, 1.0f, 1.0f});
	clearValues.push_back(vk::ClearDepthStencilValue{1.0f, 0});

	vk::RenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.renderPass = drawInfo.renderPass;
	renderPassBeginInfo.framebuffer = *(drawInfo.framebuffer);
	renderPassBeginInfo.renderArea = renderArea;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	const vk::CommandBuffer& commandBuffer = commandHandler.getCommandBuffer();

	commandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

	// Default pipeline
	const PipelineDrawData& defaultDrawInfo = drawInfo.pipelineInfos.at(PipelineType::DEFAULT);
	commandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eGraphics,
		defaultDrawInfo.layout,
		0,
		1, &(drawInfo.globalDescriptorSet),
		0, nullptr
	);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, defaultDrawInfo.pipeline);

	vk::DeviceSize offset = 0;
	commandBuffer.bindVertexBuffers(0, 1, &(drawInfo.meshLumpData.vertexBuffer), &offset);
	commandBuffer.bindIndexBuffer(drawInfo.meshLumpData.indexBuffer, 0, vk::IndexType::eUint32);

	uint32_t startInstance = 0;
	for(uint32_t i = 0; i < drawInfo.meshLumpData.indexCounts.size(); i++)
	{
		commandBuffer.bindDescriptorSets
		(
			vk::PipelineBindPoint::eGraphics,
			defaultDrawInfo.layout,
			1,
			1, &defaultDrawInfo.descriptorSets[i],
			0, nullptr
		);

		commandBuffer.drawIndexed
		(
			drawInfo.meshLumpData.indexCounts[i],
			drawInfo.meshLumpData.instanceCounts[i],
			drawInfo.meshLumpData.indexOffsets[i],
			0,
			startInstance
		);
		startInstance += drawInfo.meshLumpData.instanceCounts[i];
	}

	// Billboard pipeline
	const PipelineDrawData& billboardDrawInfo = drawInfo.pipelineInfos.at(PipelineType::BILLBOARD);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, billboardDrawInfo.pipeline);
	commandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eGraphics,
		billboardDrawInfo.layout,
		1,
		1, billboardDrawInfo.descriptorSets.data(),
		0, nullptr
	);
	commandBuffer.draw(6, 1, 0, startInstance);

	gui.renderGui(commandBuffer);

	commandBuffer.endRenderPass();

	commandHandler.endCommand();
}

// Presents frame to screen when ready
void mtd::Renderer::presentFrame
(
	const vk::SwapchainKHR& swapchain,
	const vk::Queue& presentQueue,
	const vk::Semaphore& renderFinished
) const
{
	vk::PresentInfoKHR presentInfo{};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinished;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &currentFrameIndex;
	presentInfo.pResults = nullptr;

	vk::Result result = presentQueue.presentKHR(&presentInfo);
	if(result != vk::Result::eSuccess && result != vk::Result::eErrorOutOfDateKHR)
		LOG_ERROR("Failed to present frame to screen. Vulkan result: %d", result);
}
