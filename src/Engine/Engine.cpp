#include "Engine.hpp"

#include "Utils/Logger.hpp"

#define MAX_FRAMES_IN_FLIGHT 3

mtd::Engine::Engine()
	: window{FrameDimensions{800, 600}},
	vulkanInstance{"Meltdown", VK_MAKE_API_VERSION(0, 1, 0, 0), window},
	device{vulkanInstance},
	swapchain{device, window.getDimensions(), vulkanInstance.getSurface()},
	pipeline{device.getDevice(), swapchain}
{
}

mtd::Engine::~Engine()
{
	device.getDevice().waitIdle();
}

// Begins the engine main loop
void mtd::Engine::start()
{
	uint32_t currentFrameIndex = 0;

	while(window.keepOpen())
	{
		const Frame& frame = swapchain.getFrame(currentFrameIndex);
		const vk::Fence& inFlightFence = frame.getInFlightFence();

		(void) device.getDevice().waitForFences
		(
			1, &inFlightFence, vk::True, UINT64_MAX
		);
		(void) device.getDevice().resetFences(1, &inFlightFence);

		vk::Result result = device.getDevice().acquireNextImageKHR
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
				handleWindowResize();
				currentFrameIndex = 0;
				continue;
			}
			else
			{
				LOG_ERROR("Failed to acquire swapchain image. Vulkan result: %d", result);
				break;
			}
		}

		DrawInfo drawInfo{};
		drawInfo.pipeline = &pipeline.getPipeline();
		drawInfo.renderPass = &pipeline.getRenderPass();
		drawInfo.swapchain = &swapchain.getSwapchain();
		drawInfo.extent = &swapchain.getExtent();

		swapchain.getFrame(currentFrameIndex).drawFrame(drawInfo);

		currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	}
}

// Recreates swapchain and pipeline to use new dimensions
void mtd::Engine::handleWindowResize()
{
	window.waitForValidWindowSize();
	device.getDevice().waitIdle();

	swapchain.recreate(device, window.getDimensions(), vulkanInstance.getSurface());
	pipeline.recreate(swapchain);
}
