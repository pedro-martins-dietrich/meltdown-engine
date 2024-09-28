#pragma once

#include <imgui.h>

namespace mtd
{
	// Basic structure for GUI windows
	class GuiWindow
	{
		public:
			// Exhibits the GUI window
			virtual void renderGui() = 0;

		protected:
			GuiWindow(ImVec2 winSize, ImVec2 winPos);

			// Window data
			bool showWindow;
			ImVec2 windowSize;
			ImVec2 windowPos;
	};
}
