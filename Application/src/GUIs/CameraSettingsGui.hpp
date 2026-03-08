#pragma once

#include <imgui.h>
#include <meltdown/event.hpp>
#include <meltdown/gui.hpp>

// GUI for the engine's settings
class CameraSettingsGui : public mtd::GuiWindow
{
	public:
		CameraSettingsGui();
		~CameraSettingsGui() = default;

		CameraSettingsGui(const CameraSettingsGui&) = delete;
		CameraSettingsGui& operator=(const CameraSettingsGui&) = delete;

		// Exhibits the GUI window
		virtual void renderGui() override;

	private:
		// Current size and position of the window
		ImVec2 windowSize;
		ImVec2 windowPos;

		// Camera data
		bool orthographicMode;
		float nearPlane;
		float farPlane;
		float yFOV;
		float viewWidth;
		bool updateCamera;
		bool accumulateFrames;

		// Camera mode callback handles
		mtd::EventCallbackHandle setPerspectiveCallbackHandle;
		mtd::EventCallbackHandle setOrthographicCallbackHandle;
		// Camera GUI callback handle
		mtd::EventCallbackHandle toggleGuiCallbackHandle;

		// Settings GUI
		void cameraSettingsGui();
};
