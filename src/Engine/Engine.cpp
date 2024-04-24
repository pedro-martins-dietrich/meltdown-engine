#include "Engine.hpp"

#include "Utils/Logger.hpp"

#define MAX_FRAMES_IN_FLIGHT 3

mtd::Engine::Engine()
	: window{FrameDimensions{800, 600}},
	vulkanInstance{"Meltdown", VK_MAKE_API_VERSION(0, 1, 0, 0), window},
	device{vulkanInstance},
	swapchain{device, window.getDimensions(), vulkanInstance.getSurface()},
	pipeline{device.getDevice(), swapchain},
	inputHandler{},
	camera{inputHandler, glm::vec3{0.0f, -1.0f, -4.0f}, 70.0f, window.getAspectRatio()}
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

	double lastTime;
	double currentTime = glfwGetTime();
	double frameTime = 0.016;

	DrawInfo drawInfo{};
	drawInfo.pipeline = &pipeline.getPipeline();
	drawInfo.pipelineLayout = &pipeline.getLayout();
	drawInfo.renderPass = &pipeline.getRenderPass();
	drawInfo.swapchain = &swapchain.getSwapchain();
	drawInfo.extent = &swapchain.getExtent();
	drawInfo.cameraMatrices = camera.getMatrices();

	while(window.keepOpen())
	{
		camera.setFrameTime(static_cast<float>(frameTime));
		inputHandler.handleInputs(window);
		camera.updateCamera();

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

		swapchain.getFrame(currentFrameIndex).drawFrame(drawInfo);

		currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

		lastTime = currentTime;
		currentTime = glfwGetTime();
		frameTime = glm::min(currentTime - lastTime, 1.0);
	}
}

// Recreates swapchain and pipeline to use new dimensions
void mtd::Engine::handleWindowResize()
{
	window.waitForValidWindowSize();
	device.getDevice().waitIdle();

	swapchain.recreate(device, window.getDimensions(), vulkanInstance.getSurface());
	pipeline.recreate(swapchain);
	camera.updatePerspective(70.0f, window.getAspectRatio());
}
