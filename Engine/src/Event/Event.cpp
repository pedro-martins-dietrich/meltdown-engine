#include <pch.hpp>
#include <meltdown/event.hpp>

mtd::ChangeSceneEvent::ChangeSceneEvent(const char* sceneName) : sceneName{sceneName}
{
}

const char* mtd::ChangeSceneEvent::getSceneName() const
{
	return sceneName;
}

mtd::EventType mtd::ChangeSceneEvent::getType() const
{
	return EventType::ChangeScene;
}

mtd::EventType mtd::CustomEvent::getType() const
{
	return EventType::Custom;
}
