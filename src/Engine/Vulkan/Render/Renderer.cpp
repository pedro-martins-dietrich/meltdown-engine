#include "Renderer.hpp"

#include "../../Utils/Logger.hpp"

mtd::Renderer::Renderer() : currentFrameIndex{0}
{
}

// Renders frame to screen
void mtd::Renderer::render
(
	const vk::Device& device,
	const Swapchain& swapchain,
	const Gui& gui,
	DrawInfo& drawInfo,
	bool& shouldUpdateEngine
)
{
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

	swapchain.getFrame(currentFrameIndex).drawFrame(drawInfo, gui);

	currentFrameIndex =
		shouldUpdateEngine ? 0 : (currentFrameIndex + 1) % swapchain.getFrameCount();
}
