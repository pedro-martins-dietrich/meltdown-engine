#pragma once

#include <condition_variable>

#include "Vulkan/ImGui/ImGuiHandler.hpp"
#include "Vulkan/Render/Renderer.hpp"
#include "GUIs/SettingsGui.hpp"
#include "GUIs/ProfilerGui.hpp"
#include "Camera/Camera.hpp"

// Meltdown (mtd) engine namespace
namespace mtd
{
	// Meltdown Engine main class
	class Engine
	{
		public:
			Engine(const EngineInfo& info);
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
			void run(const std::function<void(double)>& onUpdateCallback);

			// Loads a new scene, clearing the previous if necessary
			void loadScene(const char* sceneFile);

		private:
			// Engine handler objects
			Window window;
			VulkanInstance vulkanInstance;
			Device device;
			Swapchain swapchain;
			std::vector<Framebuffer> framebuffers;
			std::unique_ptr<DescriptorSetHandler> globalDescriptorSetHandler;
			CommandHandler commandHandler;
			Camera camera;
			Renderer renderer;
			ImGuiHandler imGuiHandler;

			// Pipelines
			std::vector<GraphicsPipeline> graphicsPipelines;
			std::vector<FramebufferPipeline> framebufferPipelines;
			std::vector<RayTracingPipeline> rayTracingPipelines;

			// GUIs
			SettingsGui settingsGui;
			ProfilerGui profilerGui;

			// Scene being currently rendered
			Scene scene;

			// Event callback handles
			EventCallbackHandle changeSceneCallbackHandle;
			EventCallbackHandle updateDescriptorDataCallbackHandle;

			// Flag for updating the engine
			bool shouldUpdateEngine;
			// Flag to ensure all threads finish executing
			std::atomic<bool> running;

			// Scene loading objects
			std::atomic<bool> shouldLoadScene;
			std::string sceneFileToLoad;
			std::mutex sceneLoadMutex;
			std::condition_variable sceneLoadCV;

			// Descriptors to update before rendering the next frame
			std::unordered_map<uint64_t, const void*> pendingDescriptorUpdates;
			std::mutex pendingDescriptorUpdateMutex;

			// Runs the update loop (update thread)
			void updateLoop(const std::function<void(double)>& onUpdateCallback);

			// Applies all the pending updates for the descriptors
			void updateDescriptors();

			// Sets up event callback functions
			void configureEventCallbacks();
			// Sets up descriptor set shared across pipelines
			void configureGlobalDescriptorSetHandler();
			// Creates the framebuffers and pipelines to be used in the scene
			void createRenderResources
			(
				const std::vector<FramebufferInfo>& framebufferInfos,
				const std::vector<GraphicsPipelineInfo>& graphicsPipelineInfos,
				const std::vector<FramebufferPipelineInfo>& framebufferPipelineInfos,
				const std::vector<RayTracingPipelineInfo>& rayTracingPipelineInfos
			);
			// Sets up the descriptor pools and sets
			void configureDescriptors();

			// Recreates swapchain and pipeline to apply new settings
			void updateEngine();
	};
}
