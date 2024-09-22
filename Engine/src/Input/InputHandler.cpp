#include <pch.hpp>
#include "InputHandler.hpp"

#include <meltdown/event.hpp>

#include "../Window/Window.hpp"

using ActionKeys = std::vector<mtd::KeyCode>;
static std::unordered_map<uint32_t, ActionKeys> actionMappings;
static std::unordered_map<mtd::KeyCode, bool> pressedKeys;
static std::unordered_map<uint32_t, bool> actionStatuses;

// Defines the keys that needs to be pressed to trigger an action
void mtd::NewInputHandler::mapAction(uint32_t action, std::vector<KeyCode>&& keyCodes)
{
	actionMappings[action] = std::move(keyCodes);
}

// Removes the key mapping for the action
void mtd::NewInputHandler::unmapAction(uint32_t action)
{
	actionMappings.erase(action);
}

// Adds key to the list of pressed keys
void mtd::NewInputHandler::keyPressed(KeyCode keyCode)
{
	pressedKeys[keyCode] = true;
}

// Removes key from the list of pressed keys
void mtd::NewInputHandler::keyReleased(KeyCode keyCode)
{
	pressedKeys[keyCode] = false;
}

// Dispatch start/stop action events based on the pressed keys
void mtd::NewInputHandler::checkActionEvents()
{
	for(const auto& [action, keyList]: actionMappings)
	{
		bool allKeysPressed = true;
		for(KeyCode key: keyList)
		{
			if(!pressedKeys[key])
			{
				allKeysPressed = false;
				break;
			}
		}
		if(allKeysPressed && !actionStatuses[action])
		{
			EventManager::dispatch(std::make_unique<ActionStartEvent>(action));
			actionStatuses[action] = true;
		}
		else if(!allKeysPressed && actionStatuses[action])
		{
			EventManager::dispatch(std::make_unique<ActionStopEvent>(action));
			actionStatuses[action] = false;
		}
	}
}

mtd::InputHandler::InputHandler() : context{"default"}
{
	setDefaultInputCodes();
}

// Defines a behavior for a specific input code
void mtd::InputHandler::setInputCallback
(
	const std::string& context,
	const std::string& inputCode,
	std::function<void(bool)> callback
)
{
	callbacks[context][inputCode] = callback;
}

// Handles input logic
void mtd::InputHandler::handleInputs(const Window& window)
{
	for(auto& [action, key]: keyMapping[context])
	{
		if
		(
			callbacks.find(context) == callbacks.end() ||
			callbacks.at(context).find(action) == callbacks.at(context).end()
		) continue;

		callbacks.at(context).at(action)(window.isKeyPressed(key));
	}
}

// Sets default GLFW key codes for each input code
void mtd::InputHandler::setDefaultInputCodes()
{
	keyMapping["default"]["forward"] = GLFW_KEY_W;
	keyMapping["default"]["backward"] = GLFW_KEY_S;
	keyMapping["default"]["right"] = GLFW_KEY_D;
	keyMapping["default"]["left"] = GLFW_KEY_A;
	keyMapping["default"]["up"] = GLFW_KEY_SPACE;
	keyMapping["default"]["down"] = GLFW_KEY_LEFT_CONTROL;

	keyMapping["default"]["toggle_cursor"] = GLFW_KEY_TAB;
	keyMapping["default"]["toggle_gui"] = GLFW_KEY_G;
}
