#pragma once

#include "GuiWindow.hpp"
#include "../Pipeline/Pipeline.hpp"

namespace mtd
{
	// GUI for the engine's settings
	class SettingsGui : public GuiWindow
	{
		public:
			SettingsGui(PipelineSettings& pipelineSettings, bool& shouldUpdateEngine);
			~SettingsGui() {}

			SettingsGui(const SettingsGui&) = delete;
			SettingsGui& operator=(const SettingsGui&) = delete;

			// Exhibits the GUI window
			virtual void renderGui() override;

		private:
			// References to relevant objects
			PipelineSettings& pipelineSettings;
			bool& shouldUpdateEngine;

			// Exhibit GUI
			bool showGui;

			// List of names for the selections
			std::vector<const char*> primitiveTopologyNames;
			std::vector<const char*> polygonModeNames;
			std::vector<const char*> cullModeNames;

			// Configures strings to show in the GUI
			void setNames();
	};
}
