#pragma once

#include "Window/Window.hpp"
#include "Vulkan/Frame/Swapchain.hpp"

// Meltdown (mtd) engine namespace
namespace mtd
{
	// Meltdown Engine main class
	class Engine
	{
		public:
			Engine();
			~Engine() {}

			Engine(const Engine&) = delete;
			Engine& operator=(const Engine&) = delete;

			// Begins the engine main loop
			void start();

		private:
			// Window handler
			Window window;
			// Vulkan instance handler
			VulkanInstance vulkanInstance;
			// Logical device
			Device device;
			// Swapchain
			Swapchain swapchain;
	};
}
