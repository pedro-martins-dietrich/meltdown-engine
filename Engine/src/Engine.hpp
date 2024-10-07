#pragma once

#include "Vulkan/Descriptors/DescriptorPool.hpp"
#include "Vulkan/ImGui/ImGuiHandler.hpp"
#include "Vulkan/Render/Renderer.hpp"
#include "GUIs/SettingsGui.hpp"
#include "GUIs/ProfilerGui.hpp"
#include "Camera/Camera.hpp"
#include "Scene/Scene.hpp"

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

			// Begins the engine main loop
			void run();

			// Loads a new scene, clearing the previous if necessary
			void loadScene(const char* sceneFile);

		private:
			// Engine handler objects
			Window window;
			VulkanInstance vulkanInstance;
			Device device;
			Swapchain swapchain;
			std::unordered_map<PipelineType, Pipeline> pipelines;
			std::unique_ptr<DescriptorSetHandler> globalDescriptorSetHandler;
			CommandHandler commandHandler;
			Camera camera;
			Renderer renderer;
			ImGuiHandler imGuiHandler;

			// GUIs
			SettingsGui settingsGui;
			ProfilerGui profilerGui;

			// Scene being currently rendered
			Scene scene;

			// Flag for updating the engine
			bool shouldUpdateEngine;

			// Sets up descriptor set shared across pipelines
			void configureGlobalDescriptorSetHandler();
			// Sets up the pipelines to be used
			void configurePipelines();
			// Sets up the descriptor pools and sets
			void configureDescriptors();

			// Recreates swapchain and pipeline to apply new settings
			void updateEngine();
	};
}
