#include "Window.hpp"

mtd::Window::Window(int width, int height) : glfwWindow{nullptr}, width{width}, height{height}
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

// Configures GLFW parameters
void mtd::Window::initializeGLFW()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}

// Creates GLFW window instance
void mtd::Window::createWindowInstance()
{
	if(width <= 0)
		width = 800;
	if(height <= 0)
		height = 600;

	glfwWindow = glfwCreateWindow(width, height, "Meltdown Engine", nullptr, nullptr);
}
