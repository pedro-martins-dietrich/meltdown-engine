#include <pch.hpp>
#include "Renderer.hpp"

#include "../Frame/Screenshot.hpp"
#include "../../Utils/Logger.hpp"
#include "../../Utils/Profiler.hpp"

mtd::Renderer::Renderer(const Device& mtdDevice)
	: mtdDevice{mtdDevice},
	clearValues{vk::ClearColorValue{0.1f, 0.1f, 0.1f, 1.0f}, vk::ClearDepthStencilValue{1.0f, 0U}}
{
	screenshotCallbackHandle = EventManager::addCallback([this](const ScreenshotEvent& event)
	{
		pendingScreenshot = true;
	});
}

void mtd::Renderer::setClearColor(const Vec4& color)
{
	clearValues[0] = vk::ClearColorValue{color.r, color.g, color.b, color.a};
}

void mtd::Renderer::render
(
	const Swapchain& swapchain,
	const ImGuiHandler& guiHandler,
	const std::vector<Framebuffer>& framebuffers,
	const PipelineBundle& pipelines,
	const Scene& scene,
	ResourceManager& resourceManager,
	DescriptorManager& descriptorManager,
	std::atomic<bool>& shouldUpdateEngine
)
{
	PROFILER_NEXT_STAGE("Render - Create render objects");

	std::vector<DrawBatch> drawBatches;
	renderObjectManager.createFrameRenderObjects
	(
		resourceManager, scene.getMeshes(), scene.getInstances(), drawBatches, descriptorManager
	);

	PROFILER_NEXT_STAGE("Render - Acquire frame");

	const vk::Device& device = mtdDevice.getDevice();
	const Frame& frameInFlight = swapchain.getFrame(currentFrameIndex);

	const SynchronizationBundle& syncBundle = frameInFlight.getSyncBundle();
	const CommandHandler& commandHandler = frameInFlight.getCommandHandler();

	(void) device.waitForFences(1U, &(syncBundle.inFlightFence), vk::True, UINT64_MAX);
	(void) device.resetFences(1U, &(syncBundle.inFlightFence));

	vk::Result result = device.acquireNextImageKHR
	(
		swapchain.getSwapchain(),
		UINT64_MAX,
		syncBundle.imageAvailable,
		nullptr,
		&currentImageIndex
	);
	if(result != vk::Result::eSuccess)
	{
		if
		(
			result == vk::Result::eErrorOutOfDateKHR
			|| result == vk::Result::eErrorIncompatibleDisplayKHR
			|| result == vk::Result::eSuboptimalKHR
		)
		{
			currentFrameIndex = 0U;
			shouldUpdateEngine.store(true);
		}
		else
		{
			LOG_ERROR("Failed to acquire swapchain image. Vulkan result: %d", result);
		}
		return;
	}

	const Frame& swapchainFrame = swapchain.getFrame(currentImageIndex);

	recordDrawCommands
	(
		swapchain,
		swapchainFrame.getFramebuffer(),
		framebuffers,
		pipelines,
		scene,
		resourceManager,
		commandHandler,
		drawBatches,
		guiHandler
	);
	commandHandler.submitDrawCommandBuffer(syncBundle);

	PROFILER_NEXT_STAGE("Present frame");
	vk::Semaphore presentReadySemaphore = syncBundle.renderFinished;
	if(pendingScreenshot)
	{
		Screenshot::takeScreenshot
		(
			mtdDevice, swapchainFrame.getColorBufferImage(), swapchainFrame.getDimensions(),
			presentReadySemaphore, syncBundle.screenshotCopy
		);
		presentReadySemaphore = syncBundle.screenshotCopy;
		pendingScreenshot = false;
	}

	presentFrame(swapchain.getSwapchain(), presentReadySemaphore);

	currentFrameIndex = shouldUpdateEngine.load() ? 0U : (currentFrameIndex + 1U) % swapchain.getFrameCount();
}

void mtd::Renderer::createRenderObjectsBuffer(ResourceManager& resourceManager)
{
	renderObjectManager.createBuffer(resourceManager);
}

void mtd::Renderer::configureRendererDescriptor
(
	const ResourceManager& resourceManager, DescriptorSetHandler& descriptorSetHandler
)
{
	renderObjectManager.updateDescriptor(resourceManager, descriptorSetHandler);
}

void mtd::Renderer::recordDrawCommands
(
	const Swapchain& swapchain,
	vk::Framebuffer mainFramebuffer,
	const std::vector<Framebuffer>& framebuffers,
	const PipelineBundle& pipelines,
	const Scene& scene,
	const ResourceManager& resourceManager,
	const CommandHandler& commandHandler,
	const std::vector<DrawBatch>& drawBatches,
	const ImGuiHandler& guiHandler
) const
{
	assert
	(
		!(pipelines.rasterizationPipelines.empty() && pipelines.framebufferPipelines.empty()) &&
		"There must be at least one rendering pipeline."
	);

	const vk::CommandBuffer& commandBuffer = commandHandler.getCommandBuffer();
	const vk::PipelineLayout& firstPipelineLayout =
		pipelines.rasterizationPipelines.empty()
		? pipelines.framebufferPipelines[0].getLayout()
		: pipelines.rasterizationPipelines[0].getLayout();

	commandHandler.beginCommand();

	scene.bindMeshData(resourceManager, commandBuffer);

	for(const ComputePipeline& computePipeline: pipelines.computePipelines)
	{
		PROFILER_NEXT_STAGE(computePipeline.getName().c_str());
		computePipeline.setInstanceCount(renderObjectManager.getRenderObjectCount());
		computePipeline.dispatchCompute(commandBuffer);
	}

	for(const RayTracingPipeline& rayTracingPipeline: pipelines.rayTracingPipelines)
	{
		PROFILER_NEXT_STAGE(rayTracingPipeline.getName().c_str());
		rayTracingPipeline.traceRays(commandBuffer, mtdDevice.getDLDI());
	}

	vk::Rect2D renderArea{};
	renderArea.offset = vk::Offset2D{0, 0};

	for(const RenderPassInfo& renderPassInfo: renderOrder)
	{
		int32_t fbIndex = renderPassInfo.targetFramebufferIndex;
		bool toSwapchain = (fbIndex == -1);

		renderArea.extent = toSwapchain ? swapchain.getExtent() : framebuffers[fbIndex].getExtent();

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
		renderPassBeginInfo.renderPass =
			toSwapchain ? swapchain.getRenderPass() : framebuffers[fbIndex].getRenderPass();
		renderPassBeginInfo.framebuffer = toSwapchain ? mainFramebuffer : framebuffers[fbIndex].getFramebuffer();
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
			commandBuffer.draw(3U, 1U, 0U, 0U);
		}

		renderObjectManager.bindBuffer(resourceManager, commandBuffer);
		const std::vector<MeshData>& meshes = scene.getMeshes();

		for(uint32_t pipelineIndex: renderPassInfo.pipelineIndices)
		{
			const RasterizationPipeline& rasterizationPipeline = pipelines.rasterizationPipelines[pipelineIndex];
			PROFILER_NEXT_STAGE(rasterizationPipeline.getName().c_str());

			rasterizationPipeline.bind(commandBuffer);

			for(const DrawBatch& drawBatch: drawBatches)
			{
				if(drawBatch.pipelineID != pipelineIndex) continue;
				const MeshData& mesh = meshes[drawBatch.meshID];

				for(const SubmeshData& submesh: mesh.submeshes)
				{
					rasterizationPipeline.pushConstant(commandBuffer, submesh.materialSlot);
					commandBuffer.drawIndexed
					(
						submesh.indexCount, drawBatch.instanceCount,
						submesh.indexOffset, mesh.vertexOffset,
						drawBatch.firstInstance
					);
				}
			}
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

void mtd::Renderer::presentFrame(vk::SwapchainKHR swapchain, vk::Semaphore renderFinished) const
{
	vk::PresentInfoKHR presentInfo{};
	presentInfo.waitSemaphoreCount = 1U;
	presentInfo.pWaitSemaphores = &renderFinished;
	presentInfo.swapchainCount = 1U;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &currentFrameIndex;
	presentInfo.pResults = nullptr;

	vk::Result result = mtdDevice.getPresentQueue().presentKHR(&presentInfo);
	if
	(
		result != vk::Result::eSuccess
		&& result != vk::Result::eErrorOutOfDateKHR
		&& result != vk::Result::eSuboptimalKHR
	) LOG_ERROR("Failed to present frame to screen. Vulkan result: %d", result);
}
