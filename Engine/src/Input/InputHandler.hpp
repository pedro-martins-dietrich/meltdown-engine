#pragma once

#include <vector>

#include <meltdown/enums.hpp>

// Handles the keyboard inputs
namespace mtd::InputHandler
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
