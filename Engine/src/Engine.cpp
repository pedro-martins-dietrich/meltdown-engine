#include "Engine.hpp"

#include "Utils/Logger.hpp"

mtd::Engine::Engine(const EngineInfo& info)
	: window{FrameDimensions{1280, 720}, info.appName},
	vulkanInstance{info, window},
	device{vulkanInstance},
	swapchain{device, window.getDimensions(), vulkanInstance.getSurface()},
	commandHandler{device},
	scene{device},
	inputHandler{},
	imgui{device.getDevice(), inputHandler},
	settingsGui{swapchain.getSettings(), shouldUpdateEngine},
	renderer{},
	camera{inputHandler, glm::vec3{0.0f, -1.5f, -4.5f}, 70.0f, window.getAspectRatio()},
	shouldUpdateEngine{false}
{
	configureGlobalDescriptorSetHandler();
	configurePipelines();

	window.setInputCallbacks(inputHandler);

	imgui.init
	(
		window,
		vulkanInstance.getInstance(),
		device,
		swapchain.getRenderPass(),
		swapchain.getSettings().frameCount
	);
	imgui.addGuiWindow(&settingsGui);

	LOG_INFO("Engine ready.\n");

	scene.loadScene("meltdown_demo.json", commandHandler, pipelines);

	configureDescriptors();
}

mtd::Engine::~Engine()
{
	device.getDevice().waitIdle();

	LOG_INFO("Engine shut down.");
}

// Begins the engine main loop
void mtd::Engine::start()
{
	double lastTime;
	double currentTime = glfwGetTime();
	double frameTime = 0.016;

	DrawInfo drawInfo
	{
		swapchain.getRenderPass(),
		swapchain.getExtent(),
		globalDescriptorSetHandler->getSet(0)
	};

	while(window.keepOpen())
	{
		inputHandler.handleInputs(window);
		camera.updateCamera(static_cast<float>(frameTime), window);

		scene.update(frameTime);

		renderer.render(device, swapchain, imgui, pipelines, scene, drawInfo, shouldUpdateEngine);

		if(shouldUpdateEngine)
			updateEngine();

		lastTime = currentTime;
		currentTime = glfwGetTime();
		frameTime = glm::min(currentTime - lastTime, 1.0);
	}
}

// Sets up descriptor set shared across pipelines
void mtd::Engine::configureGlobalDescriptorSetHandler()
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings(1);
	// Camera data
	bindings[0].binding = 0;
	bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex;
	bindings[0].pImmutableSamplers = nullptr;

	globalDescriptorSetHandler =
		std::make_unique<DescriptorSetHandler>(device.getDevice(), bindings);
}

// Sets up the pipelines to be used
void mtd::Engine::configurePipelines()
{
	pipelines.reserve(2);
	pipelines.emplace
	(
		std::piecewise_construct,
		std::forward_as_tuple(PipelineType::DEFAULT),
		std::forward_as_tuple
		(
			device.getDevice(),
			PipelineType::DEFAULT,
			swapchain,
			globalDescriptorSetHandler.get()
		)
	);
	pipelines.emplace
	(
		std::piecewise_construct,
		std::forward_as_tuple(PipelineType::BILLBOARD),
		std::forward_as_tuple
		(
			device.getDevice(),
			PipelineType::BILLBOARD,
			swapchain,
			globalDescriptorSetHandler.get()
		)
	);

	settingsGui.setPipelinesSettings(pipelines);
}

// Sets up the descriptors
void mtd::Engine::configureDescriptors()
{
	globalDescriptorSetHandler->clearResources();

	globalDescriptorSetHandler->defineDescriptorSetsAmount(1);
	scene.getDescriptorPool().allocateDescriptorSet(*globalDescriptorSetHandler);
	globalDescriptorSetHandler->createDescriptorResources
	(
		device, sizeof(CameraMatrices), vk::BufferUsageFlagBits::eUniformBuffer, 0, 0
	);

	void* cameraWriteLocation = globalDescriptorSetHandler->getBufferWriteLocation(0, 0);
	camera.setWriteLocation(cameraWriteLocation);

	globalDescriptorSetHandler->writeDescriptorSet(0);
}

// Recreates swapchain and pipeline to apply new settings
void mtd::Engine::updateEngine()
{
	window.waitForValidWindowSize();
	device.getDevice().waitIdle();

	swapchain.recreate(device, window.getDimensions(), vulkanInstance.getSurface());

	for(auto& [type, pipeline]: pipelines)
		pipeline.recreate(swapchain, globalDescriptorSetHandler.get());

	camera.updatePerspective(70.0f, window.getAspectRatio());

	shouldUpdateEngine = false;
}
