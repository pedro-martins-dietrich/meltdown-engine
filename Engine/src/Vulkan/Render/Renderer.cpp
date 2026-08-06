#include <pch.hpp>
#include "Renderer.hpp"

#include "../../Utils/Logger.hpp"
#include "../../Utils/Profiler.hpp"

mtd::Renderer::Renderer(const Device& mtdDevice)
	: mtdDevice{mtdDevice},
	clearValues{vk::ClearColorValue{0.1f, 0.1f, 0.1f, 1.0f}, vk::ClearDepthStencilValue{1.0f, 0U}},
	renderObjectManager{mtdDevice}
{}

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
	const ResourceManager& resourceManager,
	DescriptorSetHandler& globalDescriptorSet,
	DrawInfo& drawInfo,
	std::atomic<bool>& shouldUpdateEngine
)
{
	PROFILER_NEXT_STAGE("Render - Create render objects");

	std::vector<DrawBatch> drawBatches;
	renderObjectManager.createFrameRenderObjects
	(
		scene.getMeshPool(), scene.getInstances(), drawBatches, globalDescriptorSet
	);

	PROFILER_NEXT_STAGE("Render - Acquire frame");

	const vk::Device& device = mtdDevice.getDevice();
	const Frame& frame = swapchain.getFrame(currentFrameIndex);
	const vk::Fence& inFlightFence = frame.getInFlightFence();

	(void) device.waitForFences(1U, &inFlightFence, vk::True, UINT64_MAX);
	(void) device.resetFences(1U, &inFlightFence);

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
			currentFrameIndex = 0U;
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
		resourceManager,
		commandHandler,
		drawInfo,
		drawBatches,
		guiHandler
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

void mtd::Renderer::configureRendererDescriptor(DescriptorSetHandler& descriptorSetHandler) const
{
	renderObjectManager.createDescriptor(descriptorSetHandler, 8U);
}

void mtd::Renderer::recordDrawCommands
(
	const std::vector<Framebuffer>& framebuffers,
	const PipelineBundle& pipelines,
	const Scene& scene,
	const ResourceManager& resourceManager,
	const CommandHandler& commandHandler,
	const DrawInfo& drawInfo,
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

	if(pipelines.computePipelines.size() > 0)
	{
		commandBuffer.bindDescriptorSets
		(
			vk::PipelineBindPoint::eCompute,
			pipelines.computePipelines[0].getLayout(),
			0U,
			1U, &(drawInfo.globalDescriptorSet),
			0U, nullptr
		);
	}

	const MeshPool& meshPool = scene.getMeshPool();
	vk::DeviceSize offset{0UL};
    vk::Buffer vertexBuffer = resourceManager.getVulkanBuffer(meshPool.getVertexBufferID());
    vk::Buffer indexBuffer = resourceManager.getVulkanBuffer(meshPool.getIndexBufferID());

    commandBuffer.bindVertexBuffers(0U, 1U, &vertexBuffer, &offset);
    commandBuffer.bindIndexBuffer(indexBuffer, offset, vk::IndexType::eUint32);

	for(const ComputePipeline& computePipeline: pipelines.computePipelines)
	{
		PROFILER_NEXT_STAGE(computePipeline.getName().c_str());
		computePipeline.setInstanceCount(renderObjectManager.getRenderObjectCount());
		computePipeline.dispatchCompute(commandBuffer);
	}

	if(pipelines.rayTracingPipelines.size() > 0)
	{
		commandBuffer.bindDescriptorSets
		(
			vk::PipelineBindPoint::eRayTracingKHR,
			pipelines.rayTracingPipelines[0].getLayout(),
			0U,
			1U, &(drawInfo.globalDescriptorSet),
			0U, nullptr
		);
	}

	for(const RayTracingPipeline& rayTracingPipeline: pipelines.rayTracingPipelines)
	{
		PROFILER_NEXT_STAGE(rayTracingPipeline.getName().c_str());
		rayTracingPipeline.traceRays(commandBuffer, mtdDevice.getDLDI());
	}

	vk::Rect2D renderArea{};
	renderArea.offset = vk::Offset2D{0, 0};

	commandBuffer.bindDescriptorSets
	(
		vk::PipelineBindPoint::eGraphics,
		firstPipelineLayout,
		0U,
		1U, &(drawInfo.globalDescriptorSet),
		0U, nullptr
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
			commandBuffer.draw(3U, 1U, 0U, 0U);
		}

		renderObjectManager.bindBuffer(commandBuffer);

		for(uint32_t pipelineIndex: renderPassInfo.pipelineIndices)
		{
			const RasterizationPipeline& rasterizationPipeline = pipelines.rasterizationPipelines[pipelineIndex];
			PROFILER_NEXT_STAGE(rasterizationPipeline.getName().c_str());

			rasterizationPipeline.bind(commandBuffer);

			for(const DrawBatch& drawBatch: drawBatches)
			{
				if(drawBatch.pipelineID != pipelineIndex) continue;
				const MeshData& mesh = meshPool.getMesh(drawBatch.meshID);

				for(const SubmeshData& submesh: mesh.submeshes)
				{
					rasterizationPipeline.pushConstant(commandBuffer, submesh.materialSlot);
					commandBuffer.drawIndexed
					(
						submesh.indexCount, drawBatch.instanceCount,
						mesh.indexOffset + submesh.indexOffset, mesh.vertexOffset,
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

void mtd::Renderer::presentFrame
(
	const vk::SwapchainKHR& swapchain, const vk::Queue& presentQueue, const vk::Semaphore& renderFinished
) const
{
	vk::PresentInfoKHR presentInfo{};
	presentInfo.waitSemaphoreCount = 1U;
	presentInfo.pWaitSemaphores = &renderFinished;
	presentInfo.swapchainCount = 1U;
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
