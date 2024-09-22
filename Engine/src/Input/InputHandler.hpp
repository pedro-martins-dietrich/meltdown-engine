#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

#include <meltdown/enums.hpp>

// Handles the keyboard inputs
namespace mtd::NewInputHandler
{
	// Defines the keys that needs to be pressed to trigger an action
	void mapAction(uint32_t action, std::vector<KeyCode>&& keyCodes);
	// Removes the key mapping for the action
	void unmapAction(uint32_t action);

	// Adds key to the list of pressed keys
	void keyPressed(KeyCode keyCode);
	// Removes key from the list of pressed keys
	void keyReleased(KeyCode keyCode);

	// Dispatch start/stop action events based on the pressed keys
	void checkActionEvents();
}

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
