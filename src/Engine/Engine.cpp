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
	camera{inputHandler, glm::vec3{0.0f, -1.0f, -4.0f}, 70.0f, window.getAspectRatio()}
{
	window.setInputCallbacks(inputHandler);

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
	drawInfo.cameraMatrices = camera.getMatrices();
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

		swapchain.getFrame(currentFrameIndex).drawFrame(drawInfo);

		currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

		lastTime = currentTime;
		currentTime = glfwGetTime();
		frameTime = glm::min(currentTime - lastTime, 1.0);
	}
}

// Loads all the meshes
void mtd::Engine::loadScene()
{
	meshes.emplace_back
	(
		"ground/ground.obj",
		0,
		glm::mat4
		{
			0.7071f, 0.0f, 0.7071f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			-0.7071f, 0.0f, 0.7071f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		}
	);

	meshes.emplace_back
	(
		"polyhedra/icosahedron.obj",
		1,
		glm::mat4
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			4.0f, 0.0f, 0.0f, 1.0f
		}
	);

	meshes[1].addInstance
	(
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			-4.0f, 0.0f, 0.0f, 1.0f
		}
	);

	for(Mesh& mesh: meshes)
	{
		meshManager.loadMeshToLump(mesh);
	}
	meshManager.loadMeshesToGPU(commandHandler);

	LOG_INFO("Meshes loaded to the GPU.\n");
}

// Sets up the descriptors
void mtd::Engine::configureDescriptors()
{
	std::vector<PoolSizeData> poolSizesInfo{1};
	poolSizesInfo[0].descriptorCount = 1;
	poolSizesInfo[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	descriptorPool.createDescriptorPool(poolSizesInfo);

	DescriptorSetHandler& descriptorSetHandler = pipeline.getDescriptorSet(0);
	descriptorPool.allocateDescriptorSet(descriptorSetHandler);
	descriptorSetHandler.createDescriptorResources
	(
		device, meshManager.getModelMatricesSize(), vk::BufferUsageFlagBits::eStorageBuffer
	);

	char* bufferWriteLocation = static_cast<char*>(descriptorSetHandler.getBufferWriteLocation());
	for(Mesh& mesh: meshes)
	{
		mesh.setTransformsWriteLocation(bufferWriteLocation);
		mesh.updateTransformationMatricesDescriptor();

		bufferWriteLocation += mesh.getModelMatricesSize();
	}
	descriptorSetHandler.writeDescriptorSet();
}

// Changes the scene
void mtd::Engine::updateScene(float frameTime)
{
	meshes[1].updateTransformationMatrix
	(
		glm::rotate
		(
			meshes[1].getTransformationMatrix(0),
			frameTime,
			glm::vec3{0.0f, -1.0f, 0.0f}
		),
		0
	);
	meshes[1].updateTransformationMatrix
	(
		glm::rotate
		(
			meshes[1].getTransformationMatrix(1),
			frameTime,
			glm::vec3{0.0f, 1.0f, 0.0f}
		),
		1
	);
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
