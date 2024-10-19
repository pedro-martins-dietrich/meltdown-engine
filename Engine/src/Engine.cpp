#include <pch.hpp>
#include "Engine.hpp"

#include <meltdown/event.hpp>

#include "Utils/Logger.hpp"
#include "Utils/Profiler.hpp"
#include "Input/InputHandler.hpp"

mtd::Engine::Engine(const EngineInfo& info)
	: window{WindowInfo{1280, 720, 640, 360}, info.appName},
	vulkanInstance{info, window},
	device{vulkanInstance},
	swapchain{device, window.getDimensions(), vulkanInstance.getSurface()},
	commandHandler{device},
	scene{device},
	imGuiHandler{device.getDevice()},
	settingsGui{swapchain.getSettings(), shouldUpdateEngine},
	profilerGui{},
	renderer{},
	camera{glm::vec3{0.0f, -1.5f, -4.5f}, 70.0f, window.getAspectRatio()},
	shouldUpdateEngine{false}
{
	configureGlobalDescriptorSetHandler();
	configurePipelines();

	EventManager::addCallback(EventType::ChangeScene, [this](const Event& e)
	{
		const ChangeSceneEvent* cse = dynamic_cast<const ChangeSceneEvent*>(&e);
		loadScene(cse->getSceneName());
	});

	imGuiHandler.init
	(
		window,
		vulkanInstance.getInstance(),
		device,
		swapchain.getRenderPass(),
		swapchain.getSettings().frameCount
	);
	imGuiHandler.addGuiWindow(&settingsGui);
	#ifdef MTD_DEBUG
		imGuiHandler.addGuiWindow(&profilerGui);
	#endif

	LOG_INFO("Engine ready.\n");
}

mtd::Engine::~Engine()
{
	device.getDevice().waitIdle();

	LOG_INFO("Engine shut down.");
}

// Configures the clear color for the framebuffer
void mtd::Engine::setClearColor(const Vec4& color)
{
	renderer.setClearColor(color);
}

// Begins the engine main loop
void mtd::Engine::run()
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
		PROFILER_START_FRAME("Events");
		camera.updateCamera(static_cast<float>(frameTime), window);

		InputHandler::checkActionEvents();
		EventManager::processEvents();

		PROFILER_NEXT_STAGE("Scene update");
		scene.update(frameTime);

		renderer.render(device, swapchain, imGuiHandler, pipelines, scene, drawInfo, shouldUpdateEngine);

		if(shouldUpdateEngine)
			updateEngine();

		lastTime = currentTime;
		currentTime = glfwGetTime();
		frameTime = glm::min(currentTime - lastTime, 1.0);
		PROFILER_END_FRAME();
	}
}

// Loads a new scene, clearing the previous if necessary
void mtd::Engine::loadScene(const char* sceneFile)
{
	scene.loadScene(sceneFile, commandHandler, pipelines);
	configureDescriptors();

	scene.start();
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
	void* cameraWriteLocation = globalDescriptorSetHandler->createDescriptorResources
	(
		device, sizeof(CameraMatrices), vk::BufferUsageFlagBits::eUniformBuffer, 0, 0
	);

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
