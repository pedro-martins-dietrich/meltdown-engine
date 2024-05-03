#include "Engine.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "Utils/Logger.hpp"

#define MAX_FRAMES_IN_FLIGHT 3

mtd::Engine::Engine()
	: window{FrameDimensions{800, 600}},
	vulkanInstance{"Meltdown", VK_MAKE_API_VERSION(0, 1, 0, 0), window},
	device{vulkanInstance},
	swapchain{device, window.getDimensions(), vulkanInstance.getSurface()},
	pipeline{device.getDevice(), swapchain},
	commandHandler{device},
	meshManager{device},
	inputHandler{},
	descriptorPool{device.getDevice()},
	imgui{device.getDevice(), inputHandler},
	camera{inputHandler, glm::vec3{0.0f, -1.5f, -4.5f}, 70.0f, window.getAspectRatio()},
	scene{"meltdown_demo.json"}
{
	window.setInputCallbacks(inputHandler);

	imgui.init
	(
		window,
		vulkanInstance.getInstance(),
		device,
		pipeline.getRenderPass(),
		MAX_FRAMES_IN_FLIGHT
	);

	LOG_INFO("Engine ready.\n");

	loadScene();
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
	drawInfo.descriptorSets.push_back(pipeline.getDescriptorSet(0).getSet());

	while(window.keepOpen())
	{
		inputHandler.handleInputs(window);
		camera.updateCamera(static_cast<float>(frameTime), window);

		updateScene(frameTime);

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

		swapchain.getFrame(currentFrameIndex).drawFrame(drawInfo, imgui);

		currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

		lastTime = currentTime;
		currentTime = glfwGetTime();
		frameTime = glm::min(currentTime - lastTime, 1.0);
	}
}

// Loads all the meshes
void mtd::Engine::loadScene()
{
	for(Mesh& mesh: scene.getMeshes())
	{
		meshManager.loadMeshToLump(mesh);
	}
	meshManager.loadMeshesToGPU(commandHandler);

	LOG_INFO("Meshes loaded to the GPU.\n");
}

// Sets up the descriptors
void mtd::Engine::configureDescriptors()
{
	std::vector<PoolSizeData> poolSizesInfo{2};
	poolSizesInfo[0].descriptorCount = 1;
	poolSizesInfo[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	poolSizesInfo[1].descriptorCount = 1;
	poolSizesInfo[1].descriptorType = vk::DescriptorType::eUniformBuffer;
	descriptorPool.createDescriptorPool(poolSizesInfo);

	DescriptorSetHandler& descriptorSetHandler = pipeline.getDescriptorSet(0);
	descriptorPool.allocateDescriptorSet(descriptorSetHandler);
	descriptorSetHandler.createDescriptorResources
	(
		device, meshManager.getModelMatricesSize(), vk::BufferUsageFlagBits::eStorageBuffer, 0
	);
	descriptorSetHandler.createDescriptorResources
	(
		device, sizeof(CameraMatrices), vk::BufferUsageFlagBits::eUniformBuffer, 1
	);

	char* bufferWriteLocation = static_cast<char*>(descriptorSetHandler.getBufferWriteLocation(0));
	for(Mesh& mesh: scene.getMeshes())
	{
		mesh.setTransformsWriteLocation(bufferWriteLocation);
		mesh.updateTransformationMatricesDescriptor();

		bufferWriteLocation += mesh.getModelMatricesSize();
	}

	void* cameraWriteLocation = descriptorSetHandler.getBufferWriteLocation(1);
	camera.setWriteLocation(cameraWriteLocation);

	descriptorSetHandler.writeDescriptorSet();
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

// Recreates swapchain and pipeline to use new dimensions
void mtd::Engine::handleWindowResize()
{
	window.waitForValidWindowSize();
	device.getDevice().waitIdle();

	swapchain.recreate(device, window.getDimensions(), vulkanInstance.getSurface());
	pipeline.recreate(swapchain);
	camera.updatePerspective(70.0f, window.getAspectRatio());
}
