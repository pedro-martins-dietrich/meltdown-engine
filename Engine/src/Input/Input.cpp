#include <Meltdown.hpp>

#include "InputHandler.hpp"

void mtd::Input::mapAction(uint32_t action, std::vector<KeyCode>&& keyCodes)
{
	NewInputHandler::mapAction(action, std::move(keyCodes));
}

void mtd::Input::unmapAction(uint32_t action)
{
	NewInputHandler::unmapAction(action);
}
