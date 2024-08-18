#pragma once

#include "GuiWindow.hpp"
#include "../../Utils/EngineEnums.hpp"
#include "../Pipeline/Pipeline.hpp"

namespace mtd
{
	// GUI for the engine's settings
	class SettingsGui : public GuiWindow
	{
		public:
			SettingsGui(SwapchainSettings& swapchainSettings, bool& shouldUpdateEngine);
			~SettingsGui() {}

			SettingsGui(const SettingsGui&) = delete;
			SettingsGui& operator=(const SettingsGui&) = delete;

			// Sets the pipeline settings vector
			void setPipelinesSettings(std::unordered_map<PipelineType, Pipeline>& pipelines);

			// Exhibits the GUI window
			virtual void renderGui() override;

		private:
			// References to relevant objects
			SwapchainSettings& swapchainSettings;
			std::vector<PipelineSettings*> pipelineSettings;
			bool& shouldUpdateEngine;

			// Exhibit GUI
			bool showGui;
			// Current pipeline type index
			int pipelineTypeID;

			// List of names for the selections
			std::vector<const char*> presentModeNames;
			std::vector<const char*> primitiveTopologyNames;
			std::vector<const char*> polygonModeNames;
			std::vector<const char*> cullModeNames;

			// Settings subsections
			void swapchainSettingsGui();
			void pipelineSettingsGui();

			// Configures strings to show in the GUI
			void setNames();
	};
}
