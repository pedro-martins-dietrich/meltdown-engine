#include <pch.hpp>
#include <meltdown/event.hpp>
#include "EventManager.hpp"

using EventCallbackMap = std::unordered_map<uint64_t, mtd::EventCallback>;

static std::unordered_map<std::type_index, EventCallbackMap> callbackMaps;
static uint64_t currentCallbackIndex = 1;

static std::queue<std::unique_ptr<mtd::Event>> eventQueue;
static std::mutex eventQueueMutex;

mtd::EventCallbackHandle::EventCallbackHandle() : eventType{typeid(void)}, callbackID{0}
{
}

mtd::EventCallbackHandle::EventCallbackHandle(std::type_index eventType, uint64_t callbackID)
	: eventType{eventType}, callbackID{callbackID}
{
}

mtd::EventCallbackHandle::~EventCallbackHandle()
{
	removeCallback();
}

mtd::EventCallbackHandle::EventCallbackHandle(EventCallbackHandle&& other) noexcept
	: eventType{other.eventType}, callbackID{other.callbackID}
{
	other.callbackID = 0;
}

mtd::EventCallbackHandle& mtd::EventCallbackHandle::operator=(EventCallbackHandle&& other) noexcept
{
	if(this != &other)
	{
		eventType = other.eventType;
		callbackID = other.callbackID;
		other.callbackID = 0;
	}
	return *this;
}

void mtd::EventCallbackHandle::removeCallback()
{
	if(callbackID == 0 || callbackMaps.find(eventType) == callbackMaps.cend()) return;
	callbackMaps.at(eventType).erase(callbackID);

	callbackID = 0;
}

mtd::EventCallbackHandle mtd::EventManager::addCallback(std::type_index eventType, const EventCallback& callback)
{
	callbackMaps[eventType][currentCallbackIndex] = callback;
	return {eventType, currentCallbackIndex++};
}

void mtd::EventManager::dispatch(std::unique_ptr<Event> pEvent)
{
	std::lock_guard eventQueueLock{eventQueueMutex};
	eventQueue.push(std::move(pEvent));
}

void mtd::EventManager::processEvents()
{
	std::lock_guard eventQueueLock{eventQueueMutex};
	while(!eventQueue.empty())
	{
		std::unique_ptr<Event> pEvent = std::move(eventQueue.front());
		Event& event = *pEvent;
		eventQueue.pop();

		const std::type_index eventType{typeid(event)};
		if(callbackMaps.find(eventType) == callbackMaps.cend()) continue;

		const EventCallbackMap& callbacks = callbackMaps.at(eventType);
		for(const auto& [index, callback]: callbacks)
			callback(*pEvent);
	}
}
