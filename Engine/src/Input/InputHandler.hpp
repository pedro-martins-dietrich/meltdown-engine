#pragma once

#include <functional>
#include <map>
#include <string>

namespace mtd
{
	// Forward declaration of the window class
	class Window;

	// Handles keyboard and mouse inputs
	class InputHandler
	{
		public:
			InputHandler();
			~InputHandler() {}

			InputHandler(const InputHandler&) = delete;
			InputHandler& operator=(const InputHandler&) = delete;

			// Defines a behavior for a specific input code
			void setInputCallback
			(
				const std::string& context,
				const std::string& inputCode,
				std::function<void(bool)> callback
			);

			// Handles input logic
			void handleInputs(const Window& window);

			// Current context
			std::string context;

		private:
			using ActionKeyMap = std::map<std::string, int>;
			using ContextActionKeyMap = std::unordered_map<std::string, ActionKeyMap>;
			using ActionCallbackMap = std::unordered_map<std::string, std::function<void(bool)>>;
			using ContextActionCallbackMap = std::unordered_map<std::string, ActionCallbackMap>;

			// Assiciation between input codes and GLFW key codes
			ContextActionKeyMap keyMapping;
			// Callbacks associated to an input code
			ContextActionCallbackMap callbacks;

			// Sets default GLFW key codes for each input code
			void setDefaultInputCodes();
	};
}
