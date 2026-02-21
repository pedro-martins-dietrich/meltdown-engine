#include <pch.hpp>
#include "WindowHandler.hpp"

#include <imgui_impl_glfw.h>

#include "../Utils/Logger.hpp"
#include "../Input/InputHandler.hpp"

mtd::WindowHandler::WindowHandler(const WindowInfo& info)
	: glfwWindow{nullptr}, monitor{nullptr},
	name{info.title.c_str()},
	info{info}, aspectRatio{0.0f},
	cursorHidden{false}, fullscreenMode{false},
	shouldToggleFullscreen{false}, shouldSetInputMode{false},
	savedWindowedInfo{info}
{
	initializeGLFW();
	createWindowInstance();
	setupWindowEventDispatching();
	setWindowEventCallbacks();
}

mtd::WindowHandler::~WindowHandler()
{
	glfwTerminate();
}

bool mtd::WindowHandler::keepOpen()
{
	if(shouldToggleFullscreen)
	{
		shouldToggleFullscreen = false;
		toggleFullscreen();
	}
	if(shouldSetInputMode)
	{
		shouldSetInputMode = false;
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, cursorHidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}
	if(cursorHidden)
		glfwSetCursorPos(glfwWindow, 0.5 * info.width, 0.5 * info.height);

	glfwPollEvents();
	return !glfwWindowShouldClose(glfwWindow);
}

vk::SurfaceKHR mtd::WindowHandler::createSurface(const vk::Instance& instance) const
{
	VkSurfaceKHR surface;
	if(glfwCreateWindowSurface(instance, glfwWindow, nullptr, &surface) != VK_SUCCESS)
		LOG_ERROR("Failed to create surface for Vulkan.");

	return surface;
}

void mtd::WindowHandler::initImGuiForGLFW() const
{
	ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);
}

void mtd::WindowHandler::waitForValidWindowSize()
{
	int w = 0, h = 0;
	while(w == 0 || h == 0)
	{
		glfwGetFramebufferSize(glfwWindow, &w, &h);
		glfwWaitEvents();
	}
	aspectRatio = static_cast<float>(w) / static_cast<float>(h);

	info.width = static_cast<uint32_t>(w);
	info.height = static_cast<uint32_t>(h);

	LOG_VERBOSE("New window size: (%dx%d)", info.width, info.height);
}

void mtd::WindowHandler::getMousePos(float* x, float* y, bool needsCursorHidden) const
{
	if(needsCursorHidden && !cursorHidden) return;

	double mouseX, mouseY;
	glfwGetCursorPos(glfwWindow, &mouseX, &mouseY);
	double halfWidth = 0.5 * info.width;
	double halfHeight = 0.5 * info.height;

	*x = static_cast<float>((mouseX - halfWidth) / info.height);
	*y = static_cast<float>((mouseY - halfHeight) / info.height);
}

void mtd::WindowHandler::initializeGLFW() const
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
}

void mtd::WindowHandler::createWindowInstance()
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

void mtd::WindowHandler::setupWindowEventDispatching() const
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
		WindowHandler* pWindow = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(win));
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
		WindowHandler* pWindow = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(win));
		pWindow->info.posX = posX;
		pWindow->info.posY = posY;
	});
}

void mtd::WindowHandler::setWindowEventCallbacks()
{
	keyPressCallbackHandle = EventManager::addCallback([this](const KeyPressEvent& event)
	{
		if(event.isRepeating()) return;
		switch(event.getKeyCode())
		{
			case KeyCode::Tab:
				cursorHidden = !cursorHidden;
				shouldSetInputMode = true;
				break;
			case KeyCode::F11:
				shouldToggleFullscreen = true;
				break;
			default:
				return;
		}
	});
}

void mtd::WindowHandler::toggleFullscreen()
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
