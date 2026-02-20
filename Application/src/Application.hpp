#pragma once

#include <meltdown/window.hpp>

#include "CameraController.hpp"
#include "GUIs/CameraSettingsGui.hpp"
#include "GUIs/ProfilerGui.hpp"

// Main class responsible to interact with the engine
class Application
{
	public:
		Application();
		~Application() = default;

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		// Begins the engine main loop
		void run();

	private:
		// Window to be provided to the engine
		mtd::Window window;
		// Instance of the Meltdown Engine
		mtd::MeltdownEngine meltdownEngine;

		// Controller for the camera
		CameraController cameraController;

		// GUI for the camera settings
		CameraSettingsGui cameraSettingsGui;
		// GUI for the frame profiler
		ProfilerGui profilerGui;

		// Change scene event handle
		mtd::EventCallbackHandle changeSceneCallbackHandle;
		bool changeScene;
};
