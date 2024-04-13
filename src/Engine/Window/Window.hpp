#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

namespace mtd
{
	// GLFW window handler
	class Window
	{
		public:
			Window(int width, int height);
			~Window();

			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;

			// Poll events and checks if window should be kept open
			bool keepOpen() const;

			// Creates Vulkan surface for GLFW window
			vk::SurfaceKHR createSurface(const vk::Instance& instance) const;

		private:
			// GLFW window instance
			GLFWwindow* glfwWindow;

			// Window dimensions
			int width, height;

			// Configures GLFW parameters
			void initializeGLFW() const;
			// Creates GLFW window instance
			void createWindowInstance();
	};
}
