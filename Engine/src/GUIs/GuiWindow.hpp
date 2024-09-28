#pragma once

namespace mtd
{
	// Basic structure for GUI windows
	class GuiWindow
	{
		public:
			// Exhibits the GUI window
			virtual void renderGui() = 0;
	};
}
