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

			// Getter
			const FrameDimensions& getDimensions() const { return dimensions; }

			// Poll events and checks if window should be kept open
			bool keepOpen() const;

			// Creates Vulkan surface for GLFW window
			vk::SurfaceKHR createSurface(const vk::Instance& instance) const;

		private:
			// GLFW window instance
			GLFWwindow* glfwWindow;

			// Window dimensions
			FrameDimensions dimensions;

			// Configures GLFW parameters
			void initializeGLFW() const;
			// Creates GLFW window instance
			void createWindowInstance();
	};
}
