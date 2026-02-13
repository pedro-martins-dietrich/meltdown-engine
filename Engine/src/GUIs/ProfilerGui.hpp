#pragma once

#include <imgui.h>

#include <meltdown/gui.hpp>

namespace mtd
{
	// GUI for showing a graphic with the frame profiler data
	class ProfilerGui : public GuiWindow
	{
		public:
			ProfilerGui();
			~ProfilerGui() = default;

			ProfilerGui(const ProfilerGui&) = delete;
			ProfilerGui& operator=(const ProfilerGui&) = delete;

			// Exhibits the GUI window
			virtual void renderGui() override;

		private:
			// Current size and position of the window
			ImVec2 windowSize;
			ImVec2 windowPos;

			// Shows histogram with frame data
			void profilerGraphic();
	};
}
