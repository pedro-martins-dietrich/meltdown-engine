#include <pch.hpp>
#include "Engine.hpp"

#include "Utils/Logger.hpp"
#include "Utils/Profiler.hpp"
#include "Input/InputHandler.hpp"
#include "Event/EventManager.hpp"
#include "Vulkan/Image/SamplerManager.hpp"

mtd::Engine::Engine(const EngineInfo& info, Window& window)
	: vulkanInstance{info},
	surface{vulkanInstance.getInstance(), window.windowHandler.get()},
	device{vulkanInstance.getInstance(), surface.getSurface(), info.enableRayTracing},
	swapchain{device, surface.getSurface(), window.getDimensions()},
	commandHandler{device},
	renderer{device},
	scene{device},
	camera{window.getAspectRatio()},
	imGuiHandler{device.getDevice()},
	resourceManager{device, window.getDimensions()},
	descriptorManager{device, resourceManager}
{
	SamplerManager::createSamplers(device.getDevice());
	configureEventCallbacks();

	LOG_INFO("Engine ready.\n");
}

mtd::Engine::~Engine()
{
	device.getDevice().waitIdle();
	SamplerManager::destroySamplers(device.getDevice());

	LOG_INFO("Engine shut down.");
}

void mtd::Engine::setClearColor(const Vec4& color)
{
	renderer.setClearColor(color);
}

void mtd::Engine::setVSync(bool enableVSync)
{
	shouldUpdateEngine = swapchain.setVSync(enableVSync);
}

void mtd::Engine::run(Window& window, const std::function<void(double)>& onUpdateCallback)
{
	WindowHandler* const pWindowHandler = window.windowHandler.get();
	DrawInfo drawInfo
	{
		swapchain.getRenderPass(),
		swapchain.getExtent()
	};

	running.store(pWindowHandler->keepOpen());
	std::thread updateThread{&Engine::updateLoop, this, onUpdateCallback};

	while(running.load())
	{
		if(shouldLoadScene.load())
			loadScene(sceneFileToLoad.c_str());

		PROFILER_START_FRAME("Update descriptors");
		resourceManager.updateBufferData(cameraResourceID, sizeof(CameraMatrices), camera.fetchUpdatedMatrices());

		renderer.render
		(
			swapchain,
			imGuiHandler,
			framebuffers,
			pipelines,
			scene,
			resourceManager,
			descriptorManager,
			drawInfo,
			shouldUpdateEngine
		);

		PROFILER_NEXT_STAGE("Update engine");
		if(shouldUpdateEngine.load())
			updateEngine(pWindowHandler);

		running.store(pWindowHandler->keepOpen());
		PROFILER_END_FRAME();
	}

	if(updateThread.joinable())
		updateThread.join();
}

void mtd::Engine::loadScene(const char* sceneFile)
{
	device.getDevice().waitIdle();
	framebuffers.clear();
	pipelines.rasterizationPipelines.clear();
	pipelines.computePipelines.clear();
	pipelines.rayTracingPipelines.clear();
	pipelines.framebufferPipelines.clear();
	resourceManager.clearResources();
	descriptorManager.clear();
	Profiler::clearStages();

	std::vector<FramebufferInfo> framebufferInfos;
	PipelineInfoBundle pipelineInfos;

	cameraResourceID = resourceManager.createBuffer
	(
		"Camera", GpuBufferType::Uniform, GpuMemoryUsage::Auto, sizeof(CameraMatrices)
	);
	renderer.createRenderObjectsBuffer(resourceManager);

	scene.loadScene
	(
		device,
		sceneFile,
		framebufferInfos,
		pipelineInfos,
		resourceManager,
		descriptorManager,
		renderer.getRenderOrder()
	);

	createRenderResources(framebufferInfos, pipelineInfos);
	scene.allocateResources(pipelines);
	configureDescriptors();

	scene.start();

	shouldLoadScene.store(false);
	std::unique_lock sceneLoadLock{sceneLoadMutex};
	sceneLoadCV.notify_all();
}

void mtd::Engine::initializeGui(const Window& window)
{
	window.windowHandler->initImGuiForGLFW();
	imGuiHandler.init
	(
		vulkanInstance.getInstance(),
		device,
		swapchain.getRenderPass(),
		swapchain.getFrameCount()
	);
}

void mtd::Engine::addGuiWindow(GuiWindow* const pGuiWindow)
{
	imGuiHandler.addGuiWindow(pGuiWindow);
}

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

void mtd::Engine::configureEventCallbacks()
{
	changeSceneCallbackHandle = EventManager::addCallback([this](const ChangeSceneEvent& event)
	{
		sceneFileToLoad = event.getSceneName();
		shouldLoadScene.store(true);
	});
	windowResizeCallbackHandle = EventManager::addCallback([this](const WindowResizeEvent& event)
	{
		shouldUpdateEngine.store(true);
	});
}

void mtd::Engine::createRenderResources
(
	const std::vector<FramebufferInfo>& framebufferInfos,
	const PipelineInfoBundle& pipelineInfos
)
{
	scene.setCameraResourceID(cameraResourceID);

	framebuffers.reserve(framebufferInfos.size());
	for(const FramebufferInfo& framebufferInfo: framebufferInfos)
		framebuffers.emplace_back(device, framebufferInfo, swapchain.getExtent());

	pipelines.rasterizationPipelines.reserve(pipelineInfos.rasterizerInfos.size());
	for(const RasterizationPipelineInfo& rasterizationPipelineInfo: pipelineInfos.rasterizerInfos)
	{
		int32_t fbIndex = rasterizationPipelineInfo.targetFramebufferIndex;
		bool targetSwapchain = (fbIndex == -1);

		pipelines.rasterizationPipelines.emplace_back
		(
			device.getDevice(),
			descriptorManager,
			rasterizationPipelineInfo,
			targetSwapchain ? swapchain.getExtent() : framebuffers[fbIndex].getExtent(),
			targetSwapchain ? swapchain.getRenderPass() : framebuffers[fbIndex].getRenderPass()
		);
	}

	pipelines.framebufferPipelines.reserve(pipelineInfos.framebufferInfos.size());
	for(const FramebufferPipelineInfo& fbPipelineInfo: pipelineInfos.framebufferInfos)
	{
		int32_t fbIndex = fbPipelineInfo.targetFramebufferIndex;
		bool targetSwapchain = (fbIndex == -1);

		pipelines.framebufferPipelines.emplace_back
		(
			device.getDevice(),
			descriptorManager,
			fbPipelineInfo,
			targetSwapchain ? swapchain.getExtent() : framebuffers[fbIndex].getExtent(),
			targetSwapchain ? swapchain.getRenderPass() : framebuffers[fbIndex].getRenderPass()
		);
	}

	pipelines.computePipelines.reserve(pipelineInfos.computeInfos.size());
	for(const ComputePipelineInfo& computePipelineInfo: pipelineInfos.computeInfos)
	{
		pipelines.computePipelines.emplace_back
		(
			device, descriptorManager, computePipelineInfo, swapchain.getExtent()
		);
	}

	if(!device.isRayTracingEnabled()) return;
	pipelines.rayTracingPipelines.reserve(pipelineInfos.rayTracingInfos.size());
	for(const RayTracingPipelineInfo& rtPipelineInfo: pipelineInfos.rayTracingInfos)
	{
		pipelines.rayTracingPipelines.emplace_back
		(
			device, descriptorManager, rtPipelineInfo, swapchain.getExtent()
		);
	}
}

void mtd::Engine::configureDescriptors()
{
	for(ComputePipeline& computePipeline: pipelines.computePipelines)
		computePipeline.configurePipelineDescriptorSet();
	for(RayTracingPipeline& rtPipeline: pipelines.rayTracingPipelines)
		rtPipeline.configurePipelineDescriptorSet();
	for(FramebufferPipeline& fbPipeline: pipelines.framebufferPipelines)
		fbPipeline.updateInputImagesDescriptors(framebuffers, pipelines.computePipelines, pipelines.rayTracingPipelines);
}

void mtd::Engine::updateEngine(WindowHandler* const pWindowHandler)
{
	pWindowHandler->waitForValidWindowSize();
	device.getDevice().waitIdle();

	swapchain.recreate(device, surface.getSurface(), pWindowHandler->getDimensions());

	resourceManager.updateWindowResolutionLinkedImages({swapchain.getExtent().width, swapchain.getExtent().height});

	for(Framebuffer& framebuffer: framebuffers)
		framebuffer.resize(device, swapchain.getExtent());
	for(RasterizationPipeline& rasterizationPipeline: pipelines.rasterizationPipelines)
	{
		int32_t fbIndex = rasterizationPipeline.getTargetFramebuffer();
		if(fbIndex == -1)
			rasterizationPipeline.recreate(swapchain.getExtent(), swapchain.getRenderPass());
		else
			rasterizationPipeline.recreate(framebuffers[fbIndex].getExtent(), framebuffers[fbIndex].getRenderPass());
	}
	for(ComputePipeline& computePipeline: pipelines.computePipelines)
		computePipeline.resize(device, swapchain.getExtent());
	for(RayTracingPipeline& rayTracingPipeline: pipelines.rayTracingPipelines)
		rayTracingPipeline.resize(device, swapchain.getExtent());
	for(FramebufferPipeline& fbPipeline: pipelines.framebufferPipelines)
	{
		fbPipeline.recreate(swapchain.getExtent(), swapchain.getRenderPass());
		fbPipeline.updateInputImagesDescriptors(framebuffers, pipelines.computePipelines, pipelines.rayTracingPipelines);
	}

	camera.setAspectRatio(pWindowHandler->getAspectRatio());

	shouldUpdateEngine.store(false);
}
