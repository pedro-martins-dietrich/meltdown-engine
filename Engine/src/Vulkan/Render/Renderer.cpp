#include <pch.hpp>
#include "Renderer.hpp"

#include "../../Utils/Logger.hpp"
#include "../../Utils/Profiler.hpp"

mtd::Renderer::Renderer()
	: clearValues{vk::ClearColorValue{0.1f, 0.1f, 0.1f, 1.0f}, vk::ClearDepthStencilValue{1.0f, 0}}
{}

void mtd::Renderer::setClearColor(const Vec4& color)
{
	clearValues[0] = vk::ClearColorValue{color.r, color.g, color.b, color.a};
}

void mtd::Renderer::render
(
	const Device& mtdDevice,
	const Swapchain& swapchain,
	const ImGuiHandler& guiHandler,
	const std::vector<Framebuffer>& framebuffers,
	const PipelineBundle& pipelines,
	const Scene& scene,
	DrawInfo& drawInfo,
	std::atomic<bool>& shouldUpdateEngine
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
			shouldUpdateEngine.store(true);
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
		framebuffers,
		pipelines,
		scene,
		commandHandler,
		drawInfo,
		guiHandler,
		mtdDevice.getDLDI()
	);
	commandHandler.submitDrawCommandBuffer(*(drawInfo.syncBundle));

	PROFILER_NEXT_STAGE("Present frame");
	presentFrame
	(
		swapchain.getSwapchain(),
		mtdDevice.getPresentQueue(),
		drawInfo.syncBundle->renderFinished
	);

	currentFrameIndex = shouldUpdateEngine.load() ? 0U : (currentFrameIndex + 1U) % swapchain.getFrameCount();
}

void mtd::Renderer::recordDrawCommands
(
	const std::vector<Framebuffer>& framebuffers,
	const PipelineBundle& pipelines,
	const Scene& scene,
	const CommandHandler& commandHandler,
	const DrawInfo& drawInfo,
	const ImGuiHandler& guiHandler,
	const vk::detail::DispatchLoaderDynamic& dldi
) const
{
	assert
	(
		!(pipelines.graphicsPipelines.empty() && pipelines.framebufferPipelines.empty()) &&
		"There must be at least one rendering pipeline."
	);

	const vk::CommandBuffer& commandBuffer = commandHandler.getCommandBuffer();
	const vk::PipelineLayout& firstPipelineLayout =
		pipelines.graphicsPipelines.empty()
		? pipelines.framebufferPipelines[0].getLayout()
		: pipelines.graphicsPipelines[0].getLayout();

	commandHandler.beginCommand();

	if(pipelines.computePipelines.size() > 0)
	{
		commandBuffer.bindDescriptorSets
		(
			vk::PipelineBindPoint::eCompute,
			pipelines.computePipelines[0].getLayout(),
			0,
			1, &(drawInfo.globalDescriptorSet),
			0, nullptr
		);
	}

	for(const ComputePipeline& computePipeline: pipelines.computePipelines)
	{
		PROFILER_NEXT_STAGE(computePipeline.getName().c_str());
		computePipeline.dispatchCompute(commandBuffer);
	}

	if(pipelines.rayTracingPipelines.size() > 0)
	{
		commandBuffer.bindDescriptorSets
		(
			vk::PipelineBindPoint::eRayTracingKHR,
			pipelines.rayTracingPipelines[0].getLayout(),
			0,
			1, &(drawInfo.globalDescriptorSet),
			0, nullptr
		);
	}

	for(const RayTracingPipeline& rayTracingPipeline: pipelines.rayTracingPipelines)
	{
		PROFILER_NEXT_STAGE(rayTracingPipeline.getName().c_str());
		rayTracingPipeline.traceRays(commandBuffer, dldi);
	}

	vk::Rect2D renderArea{};
	renderArea.offset = vk::Offset2D{0, 0};

	commandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eGraphics,
		firstPipelineLayout,
		0,
		1, &(drawInfo.globalDescriptorSet),
		0, nullptr
	);

	for(const RenderPassInfo& renderPassInfo: renderOrder)
	{
		int32_t fbIndex = renderPassInfo.targetFramebufferIndex;
		bool toSwapchain = (fbIndex == -1);

		renderArea.extent = toSwapchain ? drawInfo.extent : framebuffers[fbIndex].getExtent();

		vk::ImageMemoryBarrier barrier{};
		if(renderPassInfo.framebufferPipelineIndex.has_value())
		{
			const FramebufferPipeline& fbPipeline =
				pipelines.framebufferPipelines[renderPassInfo.framebufferPipelineIndex.value()];

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
				pipelines.framebufferPipelines[renderPassInfo.framebufferPipelineIndex.value()];
			PROFILER_NEXT_STAGE(fbPipeline.getName().c_str());

			fbPipeline.bind(commandBuffer);
			commandBuffer.draw(3, 1, 0, 0);
		}

		for(uint32_t pipelineIndex: renderPassInfo.pipelineIndices)
		{
			const GraphicsPipeline& graphicsPipeline = pipelines.graphicsPipelines[pipelineIndex];
			PROFILER_NEXT_STAGE(graphicsPipeline.getName().c_str());
			const MeshManager* pMeshManager = scene.getMeshManager(pipelineIndex);

			if(pMeshManager->getMeshCount() == 0) continue;

			graphicsPipeline.bind(commandBuffer);
			pMeshManager->bindBuffers(commandBuffer);

			pMeshManager->drawMesh(commandBuffer, graphicsPipeline);
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
