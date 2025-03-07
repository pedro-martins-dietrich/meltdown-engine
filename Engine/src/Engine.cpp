#include <pch.hpp>
#include "Engine.hpp"

#include "Utils/Logger.hpp"
#include "Utils/Profiler.hpp"
#include "Input/InputHandler.hpp"
#include "Event/EventManager.hpp"

mtd::Engine::Engine(const EngineInfo& info)
	: window{WindowInfo{1280, 720, 640, 360}, info.appName},
	vulkanInstance{info, window},
	device{vulkanInstance, info.enableRayTracing},
	swapchain{device, window.getDimensions(), vulkanInstance.getSurface()},
	commandHandler{device},
	camera{window.getAspectRatio()},
	scene{device},
	imGuiHandler{device.getDevice()},
	settingsGui{swapchain.getSettings(), camera, shouldUpdateEngine},
	profilerGui{},
	renderer{},
	shouldUpdateEngine{false}, running{false},
	shouldLoadScene{false}
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

// Configures the clear color for the framebuffers
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
	DrawInfo drawInfo
	{
		swapchain.getRenderPass(),
		swapchain.getExtent(),
		globalDescriptorSetHandler->getSet(0)
	};

	running.store(window.keepOpen());
	std::thread updateThread{&Engine::updateLoop, this, onUpdateCallback};

	while(running.load())
	{
		if(shouldLoadScene.load())
			loadScene(sceneFileToLoad.c_str());

		PROFILER_START_FRAME("Update descriptors");
		updateDescriptors();
		
		renderer.render
		(
			device,
			swapchain,
			imGuiHandler,
			framebuffers,
			graphicsPipelines,
			framebufferPipelines,
			rayTracingPipelines,
			scene,
			drawInfo,
			shouldUpdateEngine
		);

		PROFILER_NEXT_STAGE("Update engine");
		if(shouldUpdateEngine)
			updateEngine();

		running.store(window.keepOpen());
		PROFILER_END_FRAME();
	}

	if(updateThread.joinable())
		updateThread.join();
}

// Loads a new scene, clearing the previous if necessary
void mtd::Engine::loadScene(const char* sceneFile)
{
	{
		std::lock_guard lock{pendingDescriptorUpdateMutex};
		pendingDescriptorUpdates.clear();
	}
	device.getDevice().waitIdle();
	framebuffers.clear();
	graphicsPipelines.clear();
	framebufferPipelines.clear();
	rayTracingPipelines.clear();
	Profiler::clearStages();

	std::vector<FramebufferInfo> framebufferInfos;
	std::vector<GraphicsPipelineInfo> graphicsPipelineInfos;
	std::vector<FramebufferPipelineInfo> framebufferPipelineInfos;
	std::vector<RayTracingPipelineInfo> rayTracingPipelineInfos;

	scene.loadScene
	(
		device,
		sceneFile,
		framebufferInfos,
		graphicsPipelineInfos,
		framebufferPipelineInfos,
		rayTracingPipelineInfos,
		renderer.getRenderOrder()
	);

	createRenderResources(framebufferInfos, graphicsPipelineInfos, framebufferPipelineInfos, rayTracingPipelineInfos);
	scene.allocateResources(graphicsPipelines, framebufferPipelines, rayTracingPipelines);
	configureDescriptors();

	scene.start();

	shouldLoadScene.store(false);
	std::unique_lock sceneLoadLock{sceneLoadMutex};
	sceneLoadCV.notify_all();
}

// Runs the update loop (update thread)
void mtd::Engine::updateLoop(const std::function<void(double)>& onUpdateCallback)
{
	using ChronoClock = std::chrono::steady_clock;
	using ChronoTime = std::chrono::steady_clock::time_point;
	using ChronoDuration = std::chrono::duration<double>;

	ChronoTime endTime = ChronoClock::now();
	ChronoDuration totalDuration{0.001};

	while(running.load())
	{
		{
			std::unique_lock sceneLoadLock{sceneLoadMutex};
			sceneLoadCV.wait(sceneLoadLock, [this] { return !shouldLoadScene.load(); });
		}

		ChronoTime startTime = endTime;

		InputHandler::checkActionEvents();
		EventManager::processEvents();

		onUpdateCallback(totalDuration.count());
		scene.update(totalDuration.count());

		endTime = ChronoClock::now();
		totalDuration = endTime - startTime;
	}
}

// Applies all the pending updates for the descriptors
void mtd::Engine::updateDescriptors()
{
	globalDescriptorSetHandler->updateDescriptorData(0, 0, camera.fetchUpdatedMatrices(), sizeof(CameraMatrices));

	std::lock_guard lock{pendingDescriptorUpdateMutex};
	for(const auto& [key, data]: pendingDescriptorUpdates)
	{
		uint32_t pipelineIndex = key >> 32;
		if(pipelineIndex >= graphicsPipelines.size()) continue;

		uint32_t binding = key & 0xFFFFFFFF;
		graphicsPipelines[pipelineIndex].updateDescriptorData(binding, data);
	}
	pendingDescriptorUpdates.clear();
}

// Sets up event callback functions
void mtd::Engine::configureEventCallbacks()
{
	changeSceneCallbackHandle = EventManager::addCallback([this](const ChangeSceneEvent& event)
	{
		sceneFileToLoad = event.getSceneName();
		shouldLoadScene.store(true);
	});
	updateDescriptorDataCallbackHandle = EventManager::addCallback([this](const UpdateDescriptorDataEvent& event)
	{
		uint64_t key = static_cast<uint64_t>(event.getPipelineIndex()) << 32 | event.getBinding();
		std::lock_guard lock{pendingDescriptorUpdateMutex};
		pendingDescriptorUpdates[key] = event.getData();
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
	bindings[0].stageFlags = vk::ShaderStageFlagBits::eAll;
	bindings[0].pImmutableSamplers = nullptr;

	globalDescriptorSetHandler = std::make_unique<DescriptorSetHandler>(device.getDevice(), bindings);
}

// Creates the framebuffers and pipelines to be used in the scene
void mtd::Engine::createRenderResources
(
	const std::vector<FramebufferInfo>& framebufferInfos,
	const std::vector<GraphicsPipelineInfo>& graphicsPipelineInfos,
	const std::vector<FramebufferPipelineInfo>& framebufferPipelineInfos,
	const std::vector<RayTracingPipelineInfo>& rayTracingPipelineInfos
)
{
	framebuffers.reserve(framebufferInfos.size());
	for(const FramebufferInfo& framebufferInfo: framebufferInfos)
		framebuffers.emplace_back(device, framebufferInfo, swapchain.getExtent());

	graphicsPipelines.reserve(graphicsPipelineInfos.size());
	for(const GraphicsPipelineInfo& graphicsPipelineInfo: graphicsPipelineInfos)
	{
		int32_t fbIndex = graphicsPipelineInfo.targetFramebufferIndex;
		bool targetSwapchain = fbIndex == -1;

		graphicsPipelines.emplace_back
		(
			device.getDevice(),
			graphicsPipelineInfo,
			globalDescriptorSetHandler->getLayout(),
			targetSwapchain ? swapchain.getExtent() : framebuffers[fbIndex].getExtent(),
			targetSwapchain ? swapchain.getRenderPass() : framebuffers[fbIndex].getRenderPass()
		);
	}

	framebufferPipelines.reserve(framebufferPipelineInfos.size());
	for(const FramebufferPipelineInfo& fbPipelineInfo: framebufferPipelineInfos)
	{
		int32_t fbIndex = fbPipelineInfo.targetFramebufferIndex;
		bool targetSwapchain = fbIndex == -1;

		framebufferPipelines.emplace_back
		(
			device.getDevice(),
			fbPipelineInfo,
			globalDescriptorSetHandler->getLayout(),
			targetSwapchain ? swapchain.getExtent() : framebuffers[fbIndex].getExtent(),
			targetSwapchain ? swapchain.getRenderPass() : framebuffers[fbIndex].getRenderPass()
		);
	}

	if(!device.isRayTracingEnabled()) return;
	rayTracingPipelines.reserve(rayTracingPipelineInfos.size());
	for(const RayTracingPipelineInfo& rtPipelineInfo: rayTracingPipelineInfos)
	{
		rayTracingPipelines.emplace_back
		(
			device, rtPipelineInfo, globalDescriptorSetHandler->getLayout(), swapchain.getExtent()
		);
	}
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

	for(GraphicsPipeline& graphicsPipeline: graphicsPipelines)
		graphicsPipeline.configureUserDescriptorData(device, scene.getDescriptorPool());
	for(RayTracingPipeline& rtPipeline: rayTracingPipelines)
	{
		rtPipeline.configureUserDescriptorData(device, scene.getDescriptorPool());
		rtPipeline.configurePipelineDescriptorSet();
	}
	for(FramebufferPipeline& fbPipeline: framebufferPipelines)
	{
		fbPipeline.configureUserDescriptorData(device, scene.getDescriptorPool());
		fbPipeline.updateInputImagesDescriptors(framebuffers, rayTracingPipelines);
	}
}

// Recreates swapchain and pipeline to apply new settings
void mtd::Engine::updateEngine()
{
	window.waitForValidWindowSize();
	device.getDevice().waitIdle();

	swapchain.recreate(device, window.getDimensions(), vulkanInstance.getSurface());

	for(Framebuffer& framebuffer: framebuffers)
		framebuffer.resize(device, swapchain.getExtent());
	for(GraphicsPipeline& graphicsPipeline: graphicsPipelines)
	{
		int32_t fbIndex = graphicsPipeline.getTargetFramebuffer();
		if(fbIndex == -1)
			graphicsPipeline.recreate(swapchain.getExtent(), swapchain.getRenderPass());
		else
			graphicsPipeline.recreate(framebuffers[fbIndex].getExtent(), framebuffers[fbIndex].getRenderPass());
	}
	for(RayTracingPipeline& rtPipeline: rayTracingPipelines)
		rtPipeline.resize(device, swapchain.getExtent());
	for(FramebufferPipeline& fbPipeline: framebufferPipelines)
	{
		fbPipeline.recreate(swapchain.getExtent(), swapchain.getRenderPass());
		fbPipeline.updateInputImagesDescriptors(framebuffers, rayTracingPipelines);
	}

	camera.setAspectRatio(window.getAspectRatio());

	shouldUpdateEngine = false;
}
