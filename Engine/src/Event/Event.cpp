#include <pch.hpp>
#include <meltdown/event.hpp>

// Key press event
mtd::KeyPressEvent::KeyPressEvent(KeyCode keyCode, bool repeatedPress)
	: keyCode{keyCode}, repeatedPress{repeatedPress}
{
}

mtd::EventType mtd::KeyPressEvent::getType() const
{
	return EventType::KeyPress;
}

mtd::KeyCode mtd::KeyPressEvent::getKeyCode() const
{
	return keyCode;
}

bool mtd::KeyPressEvent::isRepeating() const
{
	return repeatedPress;
}

// Key release event
mtd::KeyReleaseEvent::KeyReleaseEvent(KeyCode keyCode) : keyCode{keyCode}
{
}

mtd::EventType mtd::KeyReleaseEvent::getType() const
{
	return EventType::KeyRelease;
}

mtd::KeyCode mtd::KeyReleaseEvent::getKeyCode() const
{
	return keyCode;
}

// Action start event
mtd::ActionStartEvent::ActionStartEvent(uint32_t action) : action{action}
{
}

mtd::EventType mtd::ActionStartEvent::getType() const
{
	return EventType::ActionStart;
}

uint32_t mtd::ActionStartEvent::getAction() const
{
	return action;
}

//Action stop event
mtd::ActionStopEvent::ActionStopEvent(uint32_t action) : action{action}
{
}

mtd::EventType mtd::ActionStopEvent::getType() const
{
	return EventType::ActionStop;
}

uint32_t mtd::ActionStopEvent::getAction() const
{
	return action;
}

// Window position event
mtd::WindowPositionEvent::WindowPositionEvent(int posX, int posY) : posX{posX}, posY{posY}
{
}

mtd::EventType mtd::WindowPositionEvent::getType() const
{
	return EventType::WindowPosition;
}

int mtd::WindowPositionEvent::getPosX() const
{
	return posX;
}

int mtd::WindowPositionEvent::getPosY() const
{
	return posY;
}

// Change scene event
mtd::ChangeSceneEvent::ChangeSceneEvent(const char* sceneName) : sceneName{sceneName}
{
}

mtd::EventType mtd::ChangeSceneEvent::getType() const
{
	return EventType::ChangeScene;
}

const char* mtd::ChangeSceneEvent::getSceneName() const
{
	return sceneName;
}

// Change instance count event
mtd::ChangeInstanceCountEvent::ChangeInstanceCountEvent
(
	const char* modelID, int32_t instanceCountVariation
) : modelID{modelID}, instanceCountVariation{instanceCountVariation}
{
}

mtd::EventType mtd::ChangeInstanceCountEvent::getType() const
{
	return EventType::ChangeInstanceCount;
}

const std::string& mtd::ChangeInstanceCountEvent::getModelID() const
{
	return modelID;
}

int32_t mtd::ChangeInstanceCountEvent::getInstanceCountVariation() const
{
	return instanceCountVariation;
}

// Custom event
mtd::EventType mtd::CustomEvent::getType() const
{
	return EventType::Custom;
}
