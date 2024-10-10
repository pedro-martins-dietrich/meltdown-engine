#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "../Utils/EngineStructs.hpp"

namespace mtd
{
	// GLFW window handler
	class Window
	{
		public:
			Window(const WindowInfo& initialInfo, const char* windowName);
			~Window();

			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;

			// Getters
			FrameDimensions getDimensions() const;
			float getAspectRatio() const { return aspectRatio; }

			// Polls events and checks if window should be kept open
			bool keepOpen() const;
			// Checks if a specific key is pressed
			bool isKeyPressed(int glfwKeyCode) const
				{ return glfwGetKey(glfwWindow, glfwKeyCode) == GLFW_PRESS; }

			// Creates Vulkan surface for GLFW window
			vk::SurfaceKHR createSurface(const vk::Instance& instance) const;
			// Initializes ImGui GLFW backend
			void initImGuiForGLFW() const;

			// Waits until the window dimensions are valid
			void waitForValidWindowSize();

			// Returns the mouse coordinates relative to the screen center
			void getMousePos(float* x, float* y, bool needsCursorHidden) const;

		private:
			// GLFW window instance
			GLFWwindow* glfwWindow;
			// GLFW monitor instance
			GLFWmonitor* monitor;

			// Window titlebar name
			const char* name;

			// Window dimensions and position
			WindowInfo info;
			// Window aspect ratio
			float aspectRatio;

			// True if cursor is not visible
			bool cursorHidden;
			// True if window is in fullscreen mode
			bool fullscreenMode;

			// Last windowed mode settings before going fullscreen mode
			WindowInfo savedWindowedInfo;

			// Configures GLFW parameters
			void initializeGLFW() const;
			// Creates GLFW window instance
			void createWindowInstance();

			// Configures event dispatching on window callbacks
			void setupWindowEventDispatching() const;
			// Sets window input callbacks
			void setInputCallbacks();
			// Sets window event callbacks
			void setWindowEventCallbacks();

			// Toggles between fullscreen and windowed mode
			void toggleFullscreen();
	};
}
