#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "../Utils/EngineStructs.hpp"
#include "../Input/InputHandler.hpp"

namespace mtd
{
	// GLFW window handler
	class Window
	{
		public:
			Window(FrameDimensions initialDimensions);
			~Window();

			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;

			// Getters
			const FrameDimensions& getDimensions() const { return dimensions; }
			float getAspectRatio() const { return aspectRatio; }

			// Polls events and checks if window should be kept open
			bool keepOpen() const;
			// Checks if a specific key is pressed
			bool isKeyPressed(int glfwKeyCode) const
				{ return glfwGetKey(glfwWindow, glfwKeyCode) == GLFW_PRESS; }

			// Creates Vulkan surface for GLFW window
			vk::SurfaceKHR createSurface(const vk::Instance& instance) const;

			// Waits until the window dimensions are valid
			void waitForValidWindowSize();

			// Returns the mouse coordinates relative to the screen center
			void getMousePos(float* x, float* y, bool needsCursorHidden) const;

			// Sets window input callbacks
			void setInputCallbacks(InputHandler& inputHandler);

		private:
			// GLFW window instance
			GLFWwindow* glfwWindow;

			// Window dimensions
			FrameDimensions dimensions;
			// Window aspect ratio
			float aspectRatio;

			// True if cursor is not visible
			bool cursorHidden;

			// Configures GLFW parameters
			void initializeGLFW() const;
			// Creates GLFW window instance
			void createWindowInstance();
	};
}
