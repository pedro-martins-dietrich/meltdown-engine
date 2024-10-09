#include <pch.hpp>
#include "Window.hpp"

#include <imgui_impl_glfw.h>

#include <meltdown/event.hpp>

#include "../Utils/Logger.hpp"
#include "../Input/InputHandler.hpp"

mtd::Window::Window(const WindowInfo& initialInfo, const char* windowName)
	: glfwWindow{nullptr}, monitor{nullptr},
	name{windowName},
	info{initialInfo}, aspectRatio{0.0f},
	cursorHidden{false}, fullscreenMode{false},
	savedWindowedInfo{initialInfo}
{
	initializeGLFW();
	createWindowInstance();
	setupWindowEventDispatching();
	setInputCallbacks();
	setWindowEventCallbacks();
}

mtd::Window::~Window()
{
	glfwTerminate();
}

mtd::FrameDimensions mtd::Window::getDimensions() const
{
	return FrameDimensions{static_cast<uint32_t>(info.width), static_cast<uint32_t>(info.height)};
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
	info.width = 0;
	info.height = 0;
	while(info.width == 0 || info.height == 0)
	{
		glfwGetFramebufferSize(glfwWindow, &info.width, &info.height);
		glfwWaitEvents();
	}
	aspectRatio = static_cast<float>(info.width) / static_cast<float>(info.height);
	LOG_VERBOSE("New window size: (%dx%d)", info.width, info.height);
}

// Returns the mouse coordinates relative to the screen center
void mtd::Window::getMousePos(float* x, float* y, bool needsCursorHidden) const
{
	if(needsCursorHidden && !cursorHidden) return;

	double mouseX, mouseY;
	glfwGetCursorPos(glfwWindow, &mouseX, &mouseY);
	double halfWidth = 0.5f * info.width;
	double halfHeight = 0.5f * info.height;

	*x = static_cast<float>((mouseX - halfWidth) / info.height);
	*y = static_cast<float>((mouseY - halfHeight) / info.height);

	if(cursorHidden)
		glfwSetCursorPos(glfwWindow, halfWidth, halfHeight);
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
	if(info.width <= 0)
		info.width = 800;
	if(info.height <= 0)
		info.height = 600;

	aspectRatio = static_cast<float>(info.width) / static_cast<float>(info.height);

	glfwWindow = glfwCreateWindow(info.width, info.height, name, nullptr, nullptr);

	if(!glfwWindow)
	{
		LOG_ERROR("Failed to create GLFW window (%d, %d).", info.width, info.height);
		return;
	}

	monitor = glfwGetPrimaryMonitor();
	glfwGetWindowPos(glfwWindow, &info.posX, &info.posY);

	LOG_INFO("Created GLFW window with size %dx%d.", info.width, info.height);
}

// Configures event dispatching on window callbacks
void mtd::Window::setupWindowEventDispatching() const
{
	glfwSetKeyCallback(glfwWindow, [](GLFWwindow* win, int key, int scancode, int action, int mods)
	{
		KeyCode keyCode = static_cast<KeyCode>(key);
		switch(action)
		{
			case GLFW_PRESS:
				InputHandler::keyPressed(keyCode);
				EventManager::dispatch(std::make_unique<KeyPressEvent>(keyCode, false));
				break;
			case GLFW_REPEAT:
				EventManager::dispatch(std::make_unique<KeyPressEvent>(keyCode, true));
				break;
			case GLFW_RELEASE:
				InputHandler::keyReleased(keyCode);
				EventManager::dispatch(std::make_unique<KeyReleaseEvent>(keyCode));
				break;
		}
	});

	glfwSetWindowPosCallback(glfwWindow, [](GLFWwindow* win, int posX, int posY)
	{
		EventManager::dispatch(std::make_unique<WindowPositionEvent>(posX, posY));
	});
}

// Sets window input callbacks
void mtd::Window::setInputCallbacks()
{
	EventManager::addCallback(EventType::KeyPress, [this](const Event& e)
	{
		const KeyPressEvent* keyPress = dynamic_cast<const KeyPressEvent*>(&e);
		if(!keyPress || keyPress->getKeyCode() != KeyCode::Tab || keyPress->isRepeating())
			return;

		cursorHidden = !cursorHidden;
		glfwSetInputMode
		(
			glfwWindow, GLFW_CURSOR, cursorHidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL
		);
	});

	EventManager::addCallback(EventType::KeyPress, [this](const Event& e)
	{
		const KeyPressEvent* keyPress = dynamic_cast<const KeyPressEvent*>(&e);
		if(!keyPress || keyPress->getKeyCode() != KeyCode::F11 || keyPress->isRepeating())
			return;

		toggleFullscreen();
	});
}

// Sets window event callbacks
void mtd::Window::setWindowEventCallbacks()
{
	EventManager::addCallback(EventType::WindowPosition, [this](const Event& e)
	{
		const WindowPositionEvent* winPos = dynamic_cast<const WindowPositionEvent*>(&e);
		if(!winPos) return;

		info.posX = winPos->getPosX();
		info.posY = winPos->getPosY();
	});
}

// Toggles between fullscreen and windowed mode
void mtd::Window::toggleFullscreen()
{
	fullscreenMode = !fullscreenMode;
	if(fullscreenMode)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwGetWindowPos(glfwWindow, &info.posX, &info.posY);
		savedWindowedInfo = info;

		glfwSetWindowMonitor
		(
			glfwWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate
		);
	}
	else
	{
		glfwSetWindowMonitor
		(
			glfwWindow, nullptr,
			savedWindowedInfo.posX, savedWindowedInfo.posY,
			savedWindowedInfo.width, savedWindowedInfo.height, 0
		);
	}
}
