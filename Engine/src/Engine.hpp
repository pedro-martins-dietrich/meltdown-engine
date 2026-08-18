#pragma once

#include <condition_variable>

#include <meltdown/window.hpp>

#include "Camera/Camera.hpp"
#include "Vulkan/Render/Renderer.hpp"
#include "Vulkan/Frame/Surface.hpp"

// Meltdown (mtd) engine namespace
namespace mtd
{
	// Meltdown Engine main class
	class Engine
	{
		public:
			Engine(const EngineInfo& info, Window& window);
			~Engine();

			Engine(const Engine&) = delete;
			Engine& operator=(const Engine&) = delete;

			// Getters
			Camera& getCamera() { return camera; }
			bool isRayTracingEnabled() const { return device.isRayTracingEnabled(); }

			// Configures the clear color for the framebuffers
			void setClearColor(const Vec4& color);
			// Configures V-Sync
			void setVSync(bool enableVSync);

			// Begins the engine main loop
			void run(Window& window, const std::function<void(double)>& onUpdateCallback);

			// Loads a new scene, clearing the previous if necessary
			void loadScene(const char* sceneFile);

			// Initializes the engine's GUI handler
			void initializeGui(const Window& window);
			// Creates a new ImGui window
			void addGuiWindow(GuiWindow* const pGuiWindow);

		private:
			// Engine handler objects
			VulkanInstance vulkanInstance;
			Surface surface;
			Device device;
			Swapchain swapchain;
			std::vector<Framebuffer> framebuffers;
			CommandHandler commandHandler;
			Renderer renderer;
			ImGuiHandler imGuiHandler;
			ResourceManager resourceManager;
			DescriptorManager descriptorManager;

			// All pipelines in use by the scene
			PipelineBundle pipelines;

			// Scene being currently rendered
			Scene scene;

			// Scene's main camera
			Camera camera;
			ResourceID cameraResourceID = 0U;

			// Event callback handles
			EventCallbackHandle changeSceneCallbackHandle;
			EventCallbackHandle windowResizeCallbackHandle;

			// Flag for updating the engine
			std::atomic<bool> shouldUpdateEngine = false;
			// Flag to ensure all threads finish executing
			std::atomic<bool> running = false;

			// Scene loading objects
			std::atomic<bool> shouldLoadScene = false;
			std::string sceneFileToLoad;
			std::mutex sceneLoadMutex;
			std::condition_variable sceneLoadCV;

			// Runs the update loop (update thread)
			void updateLoop(const std::function<void(double)>& onUpdateCallback);

			// Sets up event callback functions
			void configureEventCallbacks();
			// Creates the framebuffers and pipelines to be used in the scene
			void createRenderResources
			(
				const std::vector<FramebufferInfo>& framebufferInfos,
				const PipelineInfoBundle& pipelineInfos
			);
			// Sets up the descriptor pools and sets
			void configureDescriptors();

			// Recreates swapchain and pipeline to apply new settings
			void updateEngine(WindowHandler* const pWindowHandler);
	};
}
