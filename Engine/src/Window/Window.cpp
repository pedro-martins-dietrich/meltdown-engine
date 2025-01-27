#include <pch.hpp>
#include "Window.hpp"

#include <imgui_impl_glfw.h>

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
	if(cursorHidden)
		glfwSetCursorPos(glfwWindow, 0.5f * info.width, 0.5f * info.height);

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

	glfwSetWindowUserPointer(glfwWindow, this);

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
				EventManager::dispatch<KeyPressEvent>(keyCode, false);
				break;
			case GLFW_REPEAT:
				EventManager::dispatch<KeyPressEvent>(keyCode, true);
				break;
			case GLFW_RELEASE:
				InputHandler::keyReleased(keyCode);
				EventManager::dispatch<KeyReleaseEvent>(keyCode);
				break;
		}
	});

	glfwSetCursorPosCallback(glfwWindow, [](GLFWwindow* win, double xPos, double yPos)
	{
		Window* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
		double halfWidth = 0.5f * pWindow->info.width;
		double halfHeight = 0.5f * pWindow->info.height;

		EventManager::dispatch<MousePositionEvent>
		(
			static_cast<float>((xPos - halfWidth) / halfHeight),
			static_cast<float>((yPos - halfHeight) / halfHeight),
			pWindow->cursorHidden
		);
	});

	glfwSetWindowPosCallback(glfwWindow, [](GLFWwindow* win, int posX, int posY)
	{
		Window* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
		pWindow->info.posX = posX;
		pWindow->info.posY = posY;
	});
}

// Configures the callbacks for window related events
void mtd::Window::setWindowEventCallbacks()
{
	keyPressCallbackHandle = EventManager::addCallback([this](const KeyPressEvent& event)
	{
		if(event.isRepeating()) return;
		switch(event.getKeyCode())
		{
			case KeyCode::Tab:
				cursorHidden = !cursorHidden;
				glfwSetInputMode(glfwWindow, GLFW_CURSOR, cursorHidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
				break;
			case KeyCode::F11:
				toggleFullscreen();
				break;
			default:
				return;
		}
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

		glfwSetWindowMonitor(glfwWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
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
