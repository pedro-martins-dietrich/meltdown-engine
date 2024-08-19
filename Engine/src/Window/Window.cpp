#include "Window.hpp"

#include <imgui_impl_glfw.h>

#include "../Utils/Logger.hpp"

mtd::Window::Window(FrameDimensions initialDimensions, const char* windowName)
	: glfwWindow{nullptr}, name{windowName}, dimensions{initialDimensions}, cursorHidden{false}
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

// Initializes ImGui GLFW backend
void mtd::Window::initImGuiForGLFW() const
{
	ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);
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
	aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	LOG_VERBOSE("New window size: (%dx%d)", dimensions.width, dimensions.height);
}

// Returns the mouse coordinates relative to the screen center
void mtd::Window::getMousePos(float* x, float* y, bool needsCursorHidden) const
{
	if(needsCursorHidden && !cursorHidden) return;

	double mouseX, mouseY;
	glfwGetCursorPos(glfwWindow, &mouseX, &mouseY);
	double halfWidth = 0.5f * dimensions.width;
	double halfHeight = 0.5f * dimensions.height;

	*x = static_cast<float>((mouseX - halfWidth) / dimensions.height);
	*y = static_cast<float>((mouseY - halfHeight) / dimensions.height);

	if(cursorHidden)
		glfwSetCursorPos(glfwWindow, halfWidth, halfHeight);
}

// Sets window input callbacks
void mtd::Window::setInputCallbacks(InputHandler& inputHandler)
{
	inputHandler.setInputCallback("default", "toggle_cursor", [this](bool pressed)
	{
		static bool lastPressed = false;

		if(!pressed || lastPressed)
		{
			lastPressed = pressed;
			return;
		}

		cursorHidden = !cursorHidden;
		glfwSetInputMode
		(
			glfwWindow, GLFW_CURSOR, cursorHidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL
		);

		lastPressed = pressed;
	});
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

	aspectRatio = static_cast<float>(dimensions.width) / static_cast<float>(dimensions.height);

	glfwWindow = glfwCreateWindow(dimensions.width, dimensions.height, name, nullptr, nullptr);

	if(glfwWindow == nullptr)
	{
		LOG_ERROR("Failed to create GLFW window (%d, %d).", dimensions.width, dimensions.height);
		return;
	}

	LOG_INFO("Created GLFW window with size %dx%d.", dimensions.width, dimensions.height);
}
