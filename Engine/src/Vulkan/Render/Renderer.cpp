#include <pch.hpp>
#include "Renderer.hpp"

#include "../../Utils/Logger.hpp"
#include "../../Utils/Profiler.hpp"

mtd::Renderer::Renderer() : currentFrameIndex{0}, clearColor{0.1f, 0.1f, 0.1f, 1.0f}
{
}

void mtd::Renderer::setClearColor(const Vec4& color)
{
	clearColor = vk::ClearColorValue{color.r, color.g, color.b, color.a};
}

// Renders frame to screen
void mtd::Renderer::render
(
	const Device& mtdDevice,
	const Swapchain& swapchain,
	const ImGuiHandler& guiHandler,
	const std::vector<Framebuffer>& framebuffers,
	const std::vector<Pipeline>& pipelines,
	const std::vector<FramebufferPipeline>& framebufferPipelines,
	const Scene& scene,
	DrawInfo& drawInfo,
	bool& shouldUpdateEngine
)
{
	PROFILER_NEXT_STAGE("Render - Acquire frame");

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
		if
		(
			result == vk::Result::eErrorOutOfDateKHR ||
			result == vk::Result::eErrorIncompatibleDisplayKHR ||
			result == vk::Result::eSuboptimalKHR
		)
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
	const CommandHandler& commandHandler = swapchain.getFrame(currentFrameIndex).getCommandHandler();

	recordDrawCommands
	(
		framebuffers, pipelines, framebufferPipelines, scene, commandHandler, drawInfo, guiHandler
	);
	commandHandler.submitDrawCommandBuffer(*(drawInfo.syncBundle));

	PROFILER_NEXT_STAGE("Present frame");
	presentFrame
	(
		swapchain.getSwapchain(),
		mtdDevice.getPresentQueue(),
		drawInfo.syncBundle->renderFinished
	);

	currentFrameIndex = shouldUpdateEngine ? 0 : (currentFrameIndex + 1) % swapchain.getFrameCount();
}

// Records draw commands to the command buffer
void mtd::Renderer::recordDrawCommands
(
	const std::vector<Framebuffer>& framebuffers,
	const std::vector<Pipeline>& pipelines,
	const std::vector<FramebufferPipeline>& framebufferPipelines,
	const Scene& scene,
	const CommandHandler& commandHandler,
	const DrawInfo& drawInfo,
	const ImGuiHandler& guiHandler
) const
{
	assert(!(pipelines.empty() && framebufferPipelines.empty()) && "There must be at least one pipeline to render.");

	const vk::CommandBuffer& commandBuffer = commandHandler.getCommandBuffer();
	const vk::PipelineLayout& firstPipelineLayout =
		pipelines.empty() ? framebufferPipelines[0].getLayout() : pipelines[0].getLayout();

	commandHandler.beginCommand();

	commandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eGraphics,
		firstPipelineLayout,
		0,
		1, &(drawInfo.globalDescriptorSet),
		0, nullptr
	);

	std::vector<vk::ClearValue> clearValues;
	clearValues.push_back(clearColor);
	clearValues.push_back(vk::ClearDepthStencilValue{1.0f, 0});

	vk::Rect2D renderArea{};
	renderArea.offset = vk::Offset2D{0, 0};

	for(const RenderPassInfo& renderPassInfo: renderOrder)
	{
		int32_t fbIndex = renderPassInfo.targetFramebufferIndex;
		bool toSwapchain = fbIndex == -1;

		renderArea.extent = toSwapchain ? drawInfo.extent : framebuffers[fbIndex].getExtent();

		vk::ImageMemoryBarrier barrier{};
		if(renderPassInfo.framebufferPipelineIndex.has_value())
		{
			const FramebufferPipeline& fbPipeline =
				framebufferPipelines[renderPassInfo.framebufferPipelineIndex.value()];

			for(AttachmentIdentifier attachmentIdentifier: fbPipeline.getAttachmentIdentifiers())
			{
				framebuffers[attachmentIdentifier.framebufferIndex]
					.transitionAttachmentLayout(true, attachmentIdentifier.attachmentIndex, barrier, commandBuffer);
			}
		}

		vk::RenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.renderPass = toSwapchain ? drawInfo.renderPass : framebuffers[fbIndex].getRenderPass();
		renderPassBeginInfo.framebuffer =
			toSwapchain ? *(drawInfo.framebuffer) : framebuffers[fbIndex].getFramebuffer();
		renderPassBeginInfo.renderArea = renderArea;
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();

		commandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

		if(renderPassInfo.framebufferPipelineIndex.has_value())
		{
			const FramebufferPipeline& fbPipeline =
				framebufferPipelines[renderPassInfo.framebufferPipelineIndex.value()];
			PROFILER_NEXT_STAGE(fbPipeline.getName().c_str());

			fbPipeline.bind(commandBuffer);
			fbPipeline.bindPipelineDescriptors(commandBuffer);
			commandBuffer.draw(3, 1, 0, 0);
		}

		for(uint32_t pipelineIndex: renderPassInfo.pipelineIndices)
		{
			const Pipeline& pipeline = pipelines[pipelineIndex];
			PROFILER_NEXT_STAGE(pipeline.getName().c_str());
			const MeshManager* pMeshManager = scene.getMeshManager(pipelineIndex);

			if(pMeshManager->getMeshCount() == 0) continue;

			pipeline.bind(commandBuffer);
			pipeline.bindPipelineDescriptors(commandBuffer);
			pMeshManager->bindBuffers(commandBuffer);

			pMeshManager->drawMesh(commandBuffer, pipeline);
		}

		if(toSwapchain)
		{
			PROFILER_NEXT_STAGE("Render - ImGUI");
			guiHandler.renderGui(commandBuffer);
		}

		commandBuffer.endRenderPass();
	}
	commandHandler.endCommand();
}

// Presents frame to screen when ready
void mtd::Renderer::presentFrame
(
	const vk::SwapchainKHR& swapchain, const vk::Queue& presentQueue, const vk::Semaphore& renderFinished
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
	if
	(
		result != vk::Result::eSuccess &&
		result != vk::Result::eErrorOutOfDateKHR &&
		result != vk::Result::eSuboptimalKHR
	) LOG_ERROR("Failed to present frame to screen. Vulkan result: %d", result);
}
