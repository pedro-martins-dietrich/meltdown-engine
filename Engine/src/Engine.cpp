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
	configureGlobalDescriptorSetHandler();

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
		swapchain.getExtent(),
		globalDescriptorSetHandler->getSet()
	};

	running.store(pWindowHandler->keepOpen());
	std::thread updateThread{&Engine::updateLoop, this, onUpdateCallback};

	while(running.load())
	{
		if(shouldLoadScene.load())
			loadScene(sceneFileToLoad.c_str());

		PROFILER_START_FRAME("Update descriptors");
		updateDescriptors();

		renderer.render
		(
			swapchain,
			imGuiHandler,
			framebuffers,
			pipelines,
			scene,
			resourceManager,
			*globalDescriptorSetHandler,
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
	{
		std::lock_guard lock{pendingDescriptorUpdateMutex};
		pendingDescriptorUpdates.clear();
	}
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

void mtd::Engine::updateDescriptors()
{
	resourceManager.updateBufferData(cameraResourceID, sizeof(CameraMatrices), camera.fetchUpdatedMatrices());

	std::lock_guard lock{pendingDescriptorUpdateMutex};
	for(const auto& [key, data]: pendingDescriptorUpdates)
	{
		uint32_t pipelineIndex = key >> 32;
		if(pipelineIndex >= pipelines.rasterizationPipelines.size()) continue;

		uint32_t binding = key & 0xFFFFFFFF;
		pipelines.rasterizationPipelines[pipelineIndex].updateDescriptorData(binding, data);
	}
	pendingDescriptorUpdates.clear();
}

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
	windowResizeCallbackHandle = EventManager::addCallback([this](const WindowResizeEvent& event)
	{
		shouldUpdateEngine.store(true);
	});
}

void mtd::Engine::configureGlobalDescriptorSetHandler()
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings(9);
	// Camera data
	bindings[0].binding = 0U;
	bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
	bindings[0].descriptorCount = 1U;
	bindings[0].stageFlags = vk::ShaderStageFlagBits::eAll;
	bindings[0].pImmutableSamplers = nullptr;
	// Scene vertex buffer
	bindings[1].binding = 1U;
	bindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[1].descriptorCount = 1U;
	bindings[1].stageFlags = vk::ShaderStageFlagBits::eAll;
	bindings[1].pImmutableSamplers = nullptr;
	// Scene index buffer
	bindings[2].binding = 2U;
	bindings[2].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[2].descriptorCount = 1U;
	bindings[2].stageFlags = vk::ShaderStageFlagBits::eAll;
	bindings[2].pImmutableSamplers = nullptr;
	// Scene submesh buffer
	bindings[3].binding = 3U;
	bindings[3].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[3].descriptorCount = 1U;
	bindings[3].stageFlags = vk::ShaderStageFlagBits::eAll;
	bindings[3].pImmutableSamplers = nullptr;
	// Scene textures data
	bindings[4].binding = 4U;
	bindings[4].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	bindings[4].descriptorCount = MAX_TEXTURE_POOL_SIZE;
	bindings[4].stageFlags = vk::ShaderStageFlagBits::eAll;
	bindings[4].pImmutableSamplers = nullptr;
	// Scene materials data
	bindings[5].binding = 5U;
	bindings[5].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[5].descriptorCount = 1U;
	bindings[5].stageFlags = vk::ShaderStageFlagBits::eAll;
	bindings[5].pImmutableSamplers = nullptr;
	// Scene materials indexing
	bindings[6].binding = 6U;
	bindings[6].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[6].descriptorCount = 1U;
	bindings[6].stageFlags = vk::ShaderStageFlagBits::eAll;
	bindings[6].pImmutableSamplers = nullptr;
	// Scene material sets
	bindings[7].binding = 7U;
	bindings[7].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[7].descriptorCount = 1U;
	bindings[7].stageFlags = vk::ShaderStageFlagBits::eAll;
	bindings[7].pImmutableSamplers = nullptr;
	// Render objects
	bindings[8].binding = 8U;
	bindings[8].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[8].descriptorCount = 1U;
	bindings[8].stageFlags = vk::ShaderStageFlagBits::eAll;
	bindings[8].pImmutableSamplers = nullptr;

	globalDescriptorSetHandler = std::make_unique<DescriptorSetHandler>(device.getDevice(), bindings);
}

void mtd::Engine::createRenderResources
(
	const std::vector<FramebufferInfo>& framebufferInfos,
	const PipelineInfoBundle& pipelineInfos
)
{
	cameraResourceID = resourceManager.createBuffer
	(
		"Camera", GpuBufferType::Uniform, GpuMemoryUsage::Auto, sizeof(CameraMatrices)
	);
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
			globalDescriptorSetHandler->getLayout(),
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
			globalDescriptorSetHandler->getLayout(),
			targetSwapchain ? swapchain.getExtent() : framebuffers[fbIndex].getExtent(),
			targetSwapchain ? swapchain.getRenderPass() : framebuffers[fbIndex].getRenderPass()
		);
	}

	pipelines.computePipelines.reserve(pipelineInfos.computeInfos.size());
	for(const ComputePipelineInfo& computePipelineInfo: pipelineInfos.computeInfos)
	{
		pipelines.computePipelines.emplace_back
		(
			device, descriptorManager, computePipelineInfo,
			globalDescriptorSetHandler->getLayout(), swapchain.getExtent()
		);
	}

	if(!device.isRayTracingEnabled()) return;
	pipelines.rayTracingPipelines.reserve(pipelineInfos.rayTracingInfos.size());
	for(const RayTracingPipelineInfo& rtPipelineInfo: pipelineInfos.rayTracingInfos)
	{
		pipelines.rayTracingPipelines.emplace_back
		(
			device, descriptorManager, rtPipelineInfo, globalDescriptorSetHandler->getLayout(), swapchain.getExtent()
		);
	}
}

void mtd::Engine::configureDescriptors()
{
	scene.getDescriptorPool().allocateDescriptorSet(*globalDescriptorSetHandler);
	scene.configureSceneDescriptorSet(resourceManager, *globalDescriptorSetHandler);
	renderer.configureRendererDescriptor(resourceManager, *globalDescriptorSetHandler);

	globalDescriptorSetHandler->writeDescriptorSet();

	for(RasterizationPipeline& rasterizationPipeline: pipelines.rasterizationPipelines)
		rasterizationPipeline.configureUserDescriptorData(device, scene.getDescriptorPool());
	for(ComputePipeline& computePipeline: pipelines.computePipelines)
	{
		computePipeline.configureUserDescriptorData(device, scene.getDescriptorPool());
		computePipeline.configurePipelineDescriptorSet();
	}
	for(RayTracingPipeline& rtPipeline: pipelines.rayTracingPipelines)
	{
		rtPipeline.configureUserDescriptorData(device, scene.getDescriptorPool());
		rtPipeline.configurePipelineDescriptorSet();
	}
	for(FramebufferPipeline& fbPipeline: pipelines.framebufferPipelines)
	{
		fbPipeline.configureUserDescriptorData(device, scene.getDescriptorPool());
		fbPipeline.updateInputImagesDescriptors(framebuffers, pipelines.computePipelines, pipelines.rayTracingPipelines);
	}
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
