#include "Engine.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "Utils/Logger.hpp"

mtd::Engine::Engine()
	: window{FrameDimensions{1280, 720}},
	vulkanInstance{"Meltdown", VK_MAKE_API_VERSION(0, 1, 0, 0), window},
	device{vulkanInstance},
	swapchain{device, window.getDimensions(), vulkanInstance.getSurface()},
	pipeline{device.getDevice(), swapchain},
	commandHandler{device},
	meshManager{device},
	inputHandler{},
	descriptorPool{device.getDevice()},
	imgui{device.getDevice(), inputHandler},
	settingsGui{swapchain.getSettings(), pipeline.getSettings(), shouldUpdateEngine},
	camera{inputHandler, glm::vec3{0.0f, -1.5f, -4.5f}, 70.0f, window.getAspectRatio()},
	shouldUpdateEngine{false}
{
	window.setInputCallbacks(inputHandler);

	imgui.init
	(
		window,
		vulkanInstance.getInstance(),
		device,
		pipeline.getRenderPass(),
		swapchain.getSettings().frameCount
	);
	imgui.addGuiWindow(&settingsGui);

	LOG_INFO("Engine ready.\n");

	scene.loadScene
	(
		"meltdown_demo.json",
		meshManager,
		commandHandler
	);

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
	uint32_t currentFrameIndex = 0;

	double lastTime;
	double currentTime = glfwGetTime();
	double frameTime = 0.016;

	DrawInfo drawInfo
	{
		MeshLumpData
		{
			meshManager.getIndexCounts(),
			meshManager.getInstanceCounts(),
			meshManager.getIndexOffsets(),
			meshManager.getVertexBuffer(),
			meshManager.getIndexBuffer()
		},
		pipeline.getPipeline(),
		pipeline.getLayout(),
		pipeline.getRenderPass(),
		swapchain.getSwapchain(),
		swapchain.getExtent(),
	};
	drawInfo.descriptorSets.push_back(pipeline.getDescriptorSetHandler(0).getSet(0));
	for(uint32_t i = 0; i < pipeline.getDescriptorSetHandler(1).getSetCount(); i++)
	{
		drawInfo.descriptorSets.push_back(pipeline.getDescriptorSetHandler(1).getSet(i));
	}

	while(window.keepOpen())
	{
		inputHandler.handleInputs(window);
		camera.updateCamera(static_cast<float>(frameTime), window);

		updateScene(frameTime);

		if(shouldUpdateEngine)
		{
			updateEngine();
			currentFrameIndex = 0;
			continue;
		}

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
				shouldUpdateEngine = true;
			}
			else
			{
				LOG_ERROR("Failed to acquire swapchain image. Vulkan result: %d", result);
				break;
			}
		}

		if(shouldUpdateEngine)
		{
			updateEngine();
			currentFrameIndex = 0;
			continue;
		}

		swapchain.getFrame(currentFrameIndex).drawFrame(drawInfo, imgui);

		currentFrameIndex = (currentFrameIndex + 1) % swapchain.getSettings().frameCount;

		lastTime = currentTime;
		currentTime = glfwGetTime();
		frameTime = glm::min(currentTime - lastTime, 1.0);
	}
}

// Sets up the descriptors
void mtd::Engine::configureDescriptors()
{
	std::vector<PoolSizeData> poolSizesInfo{3};
	poolSizesInfo[0].descriptorCount = 1;
	poolSizesInfo[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	poolSizesInfo[1].descriptorCount = 1;
	poolSizesInfo[1].descriptorType = vk::DescriptorType::eUniformBuffer;
	poolSizesInfo[2].descriptorCount = static_cast<uint32_t>(scene.getMeshes().size());
	poolSizesInfo[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descriptorPool.createDescriptorPool(poolSizesInfo);

	// Default descriptor set handler
	DescriptorSetHandler& defaultDescriptorSetHandler = pipeline.getDescriptorSetHandler(0);
	descriptorPool.allocateDescriptorSet(defaultDescriptorSetHandler);
	defaultDescriptorSetHandler.createDescriptorResources
	(
		device, meshManager.getModelMatricesSize(), vk::BufferUsageFlagBits::eStorageBuffer, 0, 0
	);
	defaultDescriptorSetHandler.createDescriptorResources
	(
		device, sizeof(CameraMatrices), vk::BufferUsageFlagBits::eUniformBuffer, 0, 1
	);

	char* bufferWriteLocation =
		static_cast<char*>(defaultDescriptorSetHandler.getBufferWriteLocation(0, 0));
	for(Mesh& mesh: scene.getMeshes())
	{
		mesh.setTransformsWriteLocation(bufferWriteLocation);
		mesh.updateTransformationMatricesDescriptor();

		bufferWriteLocation += mesh.getModelMatricesSize();
	}

	void* cameraWriteLocation = defaultDescriptorSetHandler.getBufferWriteLocation(0, 1);
	camera.setWriteLocation(cameraWriteLocation);

	defaultDescriptorSetHandler.writeDescriptorSet(0);

	// Textures descriptor set handler
	DescriptorSetHandler& texturesDescriptorSetHandler = pipeline.getDescriptorSetHandler(1);
	descriptorPool.allocateDescriptorSet(texturesDescriptorSetHandler);

	scene.loadTextures(device, commandHandler, texturesDescriptorSetHandler);
}

// Changes the scene
void mtd::Engine::updateScene(float frameTime)
{
	for(uint32_t i = 1; i < 5; i++)
	{
		Mesh& mesh = scene.getMesh(i);
		mesh.updateTransformationMatrix
		(
			glm::rotate
			(
				mesh.getTransformationMatrix(0),
				frameTime,
				glm::vec3{0.0f, -1.0f, 0.0f}
			),
			0
		);
		mesh.updateTransformationMatrix
		(
			glm::rotate
			(
				mesh.getTransformationMatrix(1),
				frameTime,
				glm::vec3{0.0f, 1.0f, 0.0f}
			),
			1
		);
	}

	Mesh& mesh = scene.getMesh(5);
	glm::mat4 matrix = mesh.getTransformationMatrix(0);
	matrix[3][0] -= frameTime * matrix[3][2];
	matrix[3][2] += frameTime * matrix[3][0];
	mesh.updateTransformationMatrix(matrix, 0);
}

// Recreates swapchain and pipeline to apply new settings
void mtd::Engine::updateEngine()
{
	window.waitForValidWindowSize();
	device.getDevice().waitIdle();

	swapchain.recreate(device, window.getDimensions(), vulkanInstance.getSurface());
	pipeline.recreate(swapchain);
	camera.updatePerspective(70.0f, window.getAspectRatio());

	shouldUpdateEngine = false;
}
