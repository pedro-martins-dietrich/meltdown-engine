#include "Window.hpp"

#include "../Utils/Logger.hpp"

mtd::Window::Window(FrameDimensions initialDimensions)
	: glfwWindow{nullptr}, dimensions{initialDimensions}
{
	initializeGLFW();
	createWindowInstance();
}

mtd::Window::~Window()
{
	glfwTerminate();
}

// Poll events and checks if window should be kept open
bool mtd::Window::keepOpen() const
{
	glfwPollEvents();
	return !glfwWindowShouldClose(glfwWindow);
}

// Creates Vulkan surface for GLFW window
vk::SurfaceKHR mtd::Window::createSurface(const vk::Instance& instance) const
{
	VkSurfaceKHR surface;
	if(glfwCreateWindowSurface(instance, glfwWindow, nullptr, &surface) != VK_SUCCESS)
		LOG_ERROR("Failed to create surface for Vulkan.");

	return surface;
}

// Waits until the window dimensions are valid
void mtd::Window::waitForValidWindowSize()
{
	int width = 0;
	int height = 0;
	while(width == 0 || height == 0)
	{
		glfwGetFramebufferSize(glfwWindow, &width, &height);
		glfwWaitEvents();
	}
	dimensions = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
	LOG_VERBOSE("New window size: (%dx%d)", dimensions.width, dimensions.height);
}

// Configures GLFW parameters
void mtd::Window::initializeGLFW() const
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
}

// Creates GLFW window instance
void mtd::Window::createWindowInstance()
{
	if(dimensions.width <= 0)
		dimensions.width = 800;
	if(dimensions.height <= 0)
		dimensions.height = 600;

	glfwWindow = glfwCreateWindow
	(
		dimensions.width, dimensions.height, "Meltdown Engine", nullptr, nullptr
	);

	if(glfwWindow == nullptr)
	{
		LOG_ERROR("Failed to create GLFW window (%d, %d).", dimensions.width, dimensions.height);
		return;
	}

	LOG_INFO("Created GLFW window with size %dx%d.", dimensions.width, dimensions.height);
}
