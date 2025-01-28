#include <pch.hpp>
#include "Engine.hpp"

#include <meltdown/event.hpp>

#include "Utils/Logger.hpp"
#include "Utils/Profiler.hpp"
#include "Input/InputHandler.hpp"
#include "Event/EventManager.hpp"

mtd::Engine::Engine(const EngineInfo& info)
	: window{WindowInfo{1280, 720, 640, 360}, info.appName},
	vulkanInstance{info, window},
	device{vulkanInstance},
	swapchain{device, window.getDimensions(), vulkanInstance.getSurface()},
	commandHandler{device},
	camera{window.getAspectRatio()},
	scene{device},
	imGuiHandler{device.getDevice()},
	settingsGui{swapchain.getSettings(), camera, shouldUpdateEngine},
	profilerGui{},
	renderer{},
	shouldUpdateEngine{false}
{
	configureEventCallbacks();
	configureGlobalDescriptorSetHandler();

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

// Configures V-Sync
void mtd::Engine::setVSync(bool enableVSync)
{
	shouldUpdateEngine = swapchain.setVSync(enableVSync);
}

// Begins the engine main loop
void mtd::Engine::run(const std::function<void(double)>& onUpdateCallback)
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
		InputHandler::checkActionEvents();
		EventManager::processEvents();

		PROFILER_NEXT_STAGE("Scene update");
		onUpdateCallback(frameTime);
		camera.updateCamera(globalDescriptorSetHandler.get());
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
	device.getDevice().waitIdle();
	pipelines.clear();
	std::vector<PipelineInfo> pipelineInfos;

	scene.loadScene(device, sceneFile, pipelineInfos);
	createPipelines(pipelineInfos);
	scene.loadMeshes(pipelines);
	configureDescriptors();

	scene.start();
}

// Sets up event callback functions
void mtd::Engine::configureEventCallbacks()
{
	changeSceneCallbackHandle = EventManager::addCallback([this](const ChangeSceneEvent& event)
	{
		loadScene(event.getSceneName());
	});
	updateDescriptorDataCallbackHandle = EventManager::addCallback([this](const UpdateDescriptorDataEvent& event)
	{
		pipelines[event.getPipelineIndex()].updateDescriptorData(event.getBinding(), event.getData());
	});
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

	globalDescriptorSetHandler = std::make_unique<DescriptorSetHandler>(device.getDevice(), bindings);
}

// Creates the pipelines to be used in the scene
void mtd::Engine::createPipelines(const std::vector<PipelineInfo>& pipelineInfos)
{
	pipelines.reserve(pipelineInfos.size());
	for(const PipelineInfo& pipelineInfo: pipelineInfos)
		pipelines.emplace_back(device.getDevice(), swapchain, globalDescriptorSetHandler.get(), pipelineInfo);
}

// Sets up the descriptors
void mtd::Engine::configureDescriptors()
{
	globalDescriptorSetHandler->defineDescriptorSetsAmount(1);
	scene.getDescriptorPool().allocateDescriptorSet(*globalDescriptorSetHandler);
	globalDescriptorSetHandler->createDescriptorResources
	(
		device, sizeof(CameraMatrices), vk::BufferUsageFlagBits::eUniformBuffer, 0, 0
	);

	globalDescriptorSetHandler->writeDescriptorSet(0);

	for(Pipeline& pipeline: pipelines)
		pipeline.configureUserDescriptorData(device, scene.getDescriptorPool());
}

// Recreates swapchain and pipeline to apply new settings
void mtd::Engine::updateEngine()
{
	window.waitForValidWindowSize();
	device.getDevice().waitIdle();

	swapchain.recreate(device, window.getDimensions(), vulkanInstance.getSurface());

	for(Pipeline& pipeline: pipelines)
		pipeline.recreate(swapchain, globalDescriptorSetHandler.get());

	camera.setAspectRatio(window.getAspectRatio());

	shouldUpdateEngine = false;
}
