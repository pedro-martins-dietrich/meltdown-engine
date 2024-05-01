#pragma once

#include "Vulkan/Pipeline/Pipeline.hpp"
#include "Vulkan/Mesh/MeshManager.hpp"
#include "Vulkan/Descriptors/DescriptorPool.hpp"
#include "Camera/Camera.hpp"
#include "Scene/Scene.hpp"

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
			DescriptorPool descriptorPool;
			Camera camera;

			// Scene being currently rendered
			Scene scene;

			// Loads all the meshes
			void loadScene();
			// Sets up the descriptor pools and sets
			void configureDescriptors();

			// Changes the scene
			void updateScene(float frameTime);

			// Recreates swapchain and pipeline to use new dimensions
			void handleWindowResize();
	};
}
