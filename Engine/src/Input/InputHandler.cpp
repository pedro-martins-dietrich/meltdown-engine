#include <pch.hpp>
#include "InputHandler.hpp"

#include <meltdown/event.hpp>

using ActionKeys = std::vector<mtd::KeyCode>;
static std::unordered_map<uint32_t, ActionKeys> actionMappings;
static std::unordered_map<mtd::KeyCode, bool> pressedKeys;
static std::mutex pressedKeysMutex;
static std::unordered_map<uint32_t, bool> actionStatuses;

// Defines the keys that needs to be pressed to trigger an action
void mtd::InputHandler::mapAction(uint32_t action, std::vector<KeyCode>&& keyCodes)
{
	actionMappings[action] = std::move(keyCodes);
}

// Removes the key mapping for the action
void mtd::InputHandler::unmapAction(uint32_t action)
{
	actionMappings.erase(action);
}

// Adds key to the list of pressed keys
void mtd::InputHandler::keyPressed(KeyCode keyCode)
{
	std::lock_guard lock{pressedKeysMutex};
	pressedKeys[keyCode] = true;
}

// Removes key from the list of pressed keys
void mtd::InputHandler::keyReleased(KeyCode keyCode)
{
	std::lock_guard lock{pressedKeysMutex};
	pressedKeys[keyCode] = false;
}

// Dispatch start/stop action events based on the pressed keys
void mtd::InputHandler::checkActionEvents()
{
	for(const auto& [action, keyList]: actionMappings)
	{
		bool allKeysPressed = true;
		for(KeyCode key: keyList)
		{
			std::lock_guard lock{pressedKeysMutex};
			if(!pressedKeys[key])
			{
				allKeysPressed = false;
				break;
			}
		}
		if(allKeysPressed && !actionStatuses[action])
		{
			EventManager::dispatch<ActionStartEvent>(action);
			actionStatuses[action] = true;
		}
		else if(!allKeysPressed && actionStatuses[action])
		{
			EventManager::dispatch<ActionStopEvent>(action);
			actionStatuses[action] = false;
		}
	}
}
