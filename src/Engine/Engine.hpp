#pragma once

#include "Window/Window.hpp"
#include "Vulkan/Pipeline/Pipeline.hpp"
#include "Camera/Camera.hpp"

// Meltdown (mtd) engine namespace
namespace mtd
{
	// Meltdown Engine main class
	class Engine
	{
		public:
			Engine();
			~Engine();

			Engine(const Engine&) = delete;
			Engine& operator=(const Engine&) = delete;

			// Begins the engine main loop
			void start();

		private:
			// Window handler
			Window window;
			// Vulkan instance handler
			VulkanInstance vulkanInstance;
			// Logical and physical device handler
			Device device;
			// Swapchain and frame handler
			Swapchain swapchain;
			// Pipeline handler
			Pipeline pipeline;

			// Camera data
			Camera camera;

			// Recreates swapchain and pipeline to use new dimensions
			void handleWindowResize();
	};
}
