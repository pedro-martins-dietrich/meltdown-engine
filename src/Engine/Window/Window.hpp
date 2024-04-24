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

		private:
			// GLFW window instance
			GLFWwindow* glfwWindow;

			// Window dimensions
			FrameDimensions dimensions;
			// Window aspect ratio
			float aspectRatio;

			// Configures GLFW parameters
			void initializeGLFW() const;
			// Creates GLFW window instance
			void createWindowInstance();
	};
}
