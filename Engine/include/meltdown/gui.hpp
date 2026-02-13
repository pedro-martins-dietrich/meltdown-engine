#pragma once

namespace mtd
{
	/*
	* @brief Base class for custom ImGui windows.
	*/
	class GuiWindow
	{
		public:
			/*
			* @brief Virtual function for ImGui window rendering.
			* Implementation should include `ImGui::Begin()` and `ImGui::End()`.
			*/
			virtual void renderGui() = 0;

			/*
			* @brief Getter for the current GUI window state.
			*
			* @return If the window is active and visible, returns `true`.
			*/
			bool isWindowActive() const { return showWindow; }

		protected:
			/*
			* @brief Constructor for the GUI window.
			*/
			GuiWindow();

			/* @brief Flag to indicate if the GUI window is active. */
			bool showWindow;
	};
}
