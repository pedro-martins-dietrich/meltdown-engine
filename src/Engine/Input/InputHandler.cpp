#include "InputHandler.hpp"

#include "../Window/Window.hpp"

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
}
