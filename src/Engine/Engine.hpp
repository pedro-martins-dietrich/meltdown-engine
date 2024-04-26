#pragma once

#include "Vulkan/Pipeline/Pipeline.hpp"
#include "Vulkan/Mesh/MeshManager.hpp"
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
			// Engine's handler objects
			Window window;
			VulkanInstance vulkanInstance;
			Device device;
			Swapchain swapchain;
			Pipeline pipeline;
			CommandHandler commandHandler;
			MeshManager meshManager;
			InputHandler inputHandler;
			Camera camera;

			// Recreates swapchain and pipeline to use new dimensions
			void handleWindowResize();
	};
}
