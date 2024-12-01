#pragma once

#include "GuiWindow.hpp"
#include "../Vulkan/Frame/Swapchain.hpp"

namespace mtd
{
	// GUI for the engine's settings
	class SettingsGui : public GuiWindow
	{
		public:
			SettingsGui(SwapchainSettings& swapchainSettings, bool& shouldUpdateEngine);
			~SettingsGui() = default;

			SettingsGui(const SettingsGui&) = delete;
			SettingsGui& operator=(const SettingsGui&) = delete;

			// Exhibits the GUI window
			virtual void renderGui() override;

		private:
			// References to relevant objects
			SwapchainSettings& swapchainSettings;
			bool& shouldUpdateEngine;

			// Settings subsection
			void swapchainSettingsGui();
	};
}
