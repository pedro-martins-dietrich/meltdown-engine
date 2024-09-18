#pragma once

#include <functional>
#include <memory>

namespace mtd
{
	/*
	* @brief Enumerations of the types of events handled by the engine.
	* New types of events should use the `Custom` variant.
	*/
	enum class EventType
	{
		ChangeScene,
		Custom
	};

	/*
	* @brief Base class for any event. An event object contains data about the event itself,
	* and should be dispatched to the `EventManager`, allowing the callbacks listening to the
	* event to be called.
	*/
	class Event
	{
		public:
			virtual ~Event() = default;

			/*
			* @brief Getter for the event type, for identification.
			* 
			* @return Kind of event the event object contains.
			*/
			virtual EventType getType() const = 0;
	};

	/*
	* @brief Event class for changing the current scene. Dispatching it will load the scene with
	* the specified scene name.
	*/
	class ChangeSceneEvent : public Event
	{
		public:
			/*
			* @brief Creates an event to change the scene.
			* 
			* @param sceneName Scene file name.
			*/
			ChangeSceneEvent(const char* sceneName);

			virtual EventType getType() const override;

			/*
			* @brief Getter for the name of the new scene.
			* 
			* @return String with the scene file name.
			*/
			const char* getSceneName() const;

		private:
			const char* sceneName;
	};

	/*
	* @brief Base class for custom events. Each custom event must have an unique ID.
	*/
	class CustomEvent : public Event
	{
		public:
			virtual EventType getType() const override;

			/*
			* @brief Getter that returns the ID of the custom event.
			* It is recommended for the application to create an `enum` to handle the IDs.
			*/
			virtual uint64_t getID() const = 0;
	};

	/*
	* @brief Type for callback functions that accept a const reference to an `Event` as parameter
	* and does not have a return value.
	*/
	using EventCallback = std::function<void(const Event&)>;

	/*
	* @brief Handles the event dispatches and callbacks of the engine.
	*/
	namespace EventManager
	{
		/*
		* @brief Adds a callback function to the specified event type.
		* The callbacks must be removed with `removeCallback()` as soon as it is no longer used.
		*
		* @param eventType Enum identifying the type of event to listen.
		* @param callback Callback function to be called when the event is triggered.
		* 
		* @return Callback ID where the callback was saved. Required for removing the callback
		* once it is no longer used.
		*/
		uint64_t addCallback(EventType eventType, EventCallback callback);
		/*
		* @brief Adds a callbacks to the specified custom event.
		* The callbacks must be removed with `removeCallback()` as soon as it is no longer used.
		*
		* @param customEventID ID identifying the custom event to listen.
		* @param callback Callback function to be called when the custom event is triggered.
		* 
		* @return Callback ID where the callback was saved. Required for removing the callback
		* once it is no longer used.
		*/
		uint64_t addCallback(uint64_t customEventID, EventCallback callback);

		/*
		* @brief Removes an event callback from the handler, specified by the event type and
		* the callback ID.
		* 
		* If the callback depends on an object that got deleted, the callback must be deleted
		* from the `EventManager`.
		* 
		* @param eventType Enum identifying the type of event of the callback.
		* @param callbackID Identification of the callback to be removed.
		*/
		void removeCallback(EventType eventType, uint64_t callbackID);
		/*
		* @brief Removes an custom event callback from the handler, specified by the
		* custom event ID and the callback ID.
		*
		* If the callback depends on an object that got deleted, the callback must be deleted
		* from the `EventManager`.
		*
		* @param eventType Enum identifying the type of event of the callback.
		* @param callbackID Identification of the callback to be removed.
		*/
		void removeCallback(uint64_t customEventID, uint64_t callbackID);

		/*
		* @brief Dispatches an event to be handled.
		*
		* @param pEvent Unique pointer to an event, transfering the pointer ownership to the
		* `EventManager`.
		*/
		void dispatch(std::unique_ptr<Event> pEvent);

		/*
		* @brief Executes all callbacks related to the queued (dispatched) events.
		* Used internaly by the engine.
		*/
		void processEvents();
	};
}
