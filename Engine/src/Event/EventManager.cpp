#include <pch.hpp>
#include <meltdown/event.hpp>
#include "EventManager.hpp"

using EventCallbackMap = std::unordered_map<uint64_t, mtd::EventCallback>;

static std::unordered_map<mtd::EventType, EventCallbackMap> callbackMaps;
static uint64_t currentCallbackIndex = 0;

static std::unordered_map<uint64_t, EventCallbackMap> customCallbackMaps;
static uint64_t currentCustomCallbackIndex = 0;

static std::queue<std::unique_ptr<mtd::Event>> eventQueue;

uint64_t mtd::EventManager::addCallback(EventType eventType, EventCallback callback)
{
	callbackMaps[eventType][currentCallbackIndex] = callback;

	return currentCallbackIndex++;
}

uint64_t mtd::EventManager::addCallback(uint64_t customEventID, EventCallback callback)
{
	customCallbackMaps[customEventID][currentCustomCallbackIndex] = callback;

	return currentCustomCallbackIndex++;
}

void mtd::EventManager::removeCallback(mtd::EventType eventType, uint64_t callbackID)
{
	callbackMaps[eventType].erase(callbackID);
}

void mtd::EventManager::removeCallback(uint64_t customEventID, uint64_t callbackID)
{
	customCallbackMaps[customEventID].erase(callbackID);
}

void mtd::EventManager::dispatch(std::unique_ptr<Event> pEvent)
{
	eventQueue.push(std::move(pEvent));
}

void mtd::EventManager::processEvents()
{
	while(!eventQueue.empty())
	{
		std::unique_ptr<Event> pEvent = std::move(eventQueue.front());
		eventQueue.pop();

		if(pEvent->getType() != EventType::Custom)
		{
			const EventCallbackMap& callbacks = callbackMaps[pEvent->getType()];
			for(const auto& [index, callback] : callbacks)
				callback(*pEvent);
		}
		else
		{
			const CustomEvent& customEvent = static_cast<const CustomEvent&>(*pEvent);
			const EventCallbackMap& callbacks = customCallbackMaps[customEvent.getID()];
			for(const auto& [index, callback] : callbacks)
				callback(customEvent);
		}
	}
}
