#pragma once

#include "GuiWindow.hpp"
#include "../Camera/Camera.hpp"

namespace mtd
{
	// GUI for the engine's settings
	class SettingsGui : public GuiWindow
	{
		public:
			SettingsGui(SwapchainSettings& swapchainSettings, const Camera& camera, bool& shouldUpdateEngine);
			~SettingsGui() = default;

			SettingsGui(const SettingsGui&) = delete;
			SettingsGui& operator=(const SettingsGui&) = delete;

			// Exhibits the GUI window
			virtual void renderGui() override;

		private:
			// References to swapchain data
			SwapchainSettings& swapchainSettings;
			bool& shouldUpdateEngine;

			// Camera data
			bool orthographicMode;
			float nearPlane;
			float farPlane;
			float yFOV;
			float viewWidth;
			bool updateCamera;

			// Settings subsections
			void swapchainSettingsGui();
			void cameraSettingsGui();
	};
}
