#pragma once

#include <functional>
#include <memory>
#include <string>

#include <meltdown/enums.hpp>

namespace mtd
{
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
	* @brief Event created every time a physical key is pressed or held down.
	*/
	class KeyPressEvent : public Event
	{
		public:
			/*
			* @brief Creates a key press event with the relevant data.
			*
			* @param keyCode Enumeration indicating which key has been pressed or held.
			* @param repeatedPress Indicates if the the key has been just pressed (`false`),
			* or if it is being held down (`true`).
			*/
			KeyPressEvent(KeyCode keyCode, bool repeatedPress);

			virtual EventType getType() const override;

			/*
			* @brief Getter for the pressed key code.
			*
			* @return Enumeration indicating which key was pressed.
			*/
			KeyCode getKeyCode() const;
			/*
			* @brief Getter for checking if the key is being continuously pressed.
			*
			* @return Indicates if the the key has been just pressed (`false`),
			* or if it is being held down (`true`).
			*/
			bool isRepeating() const;

		private:
			KeyCode keyCode;
			bool repeatedPress;
	};

	/*
	* @brief Event created every time a physical key is released.
	*/
	class KeyReleaseEvent : public Event
	{
		public:
			/*
			* @brief Creates a key release event with the relevant data.
			*
			* @param keyCode Enumeration indicating which key has been released.
			*/
			KeyReleaseEvent(KeyCode keyCode);

			virtual EventType getType() const override;

			/*
			* @brief Getter for the released key code.
			*
			* @return Enumeration indicating which key was released.
			*/
			KeyCode getKeyCode() const;

		private:
			KeyCode keyCode;
	};

	/*
	* @brief Event created when an action is started, by pressing all keys mapped to that action.
	*/
	class ActionStartEvent : public Event
	{
		public:
			/*
			* @brief Creates an event indicating that a certain action has started.
			* 
			* @param action Identifier of the action.
			*/
			ActionStartEvent(uint32_t action);

			virtual EventType getType() const override;

			/*
			* @brief Getter for the action that has been started.
			*
			* @return Value corresponding to the enum or ID of the action.
			*/
			uint32_t getAction() const;

		private:
			uint32_t action;
	};

	/*
	* @brief Event created when the action stops, once not all mapped keys are pressed.
	*/
	class ActionStopEvent : public Event
	{
		public:
			/*
			* @brief Creates an event indicating that a certain action has stopped.
			*
			* @param action Identifier of the action.
			*/
			ActionStopEvent(uint32_t action);

			virtual EventType getType() const override;

			/*
			* @brief Getter for the action that has been stopped.
			*
			* @return Value corresponding to the enum or ID of the action.
			*/
			uint32_t getAction() const;

		private:
			uint32_t action;
	};

	/*
	* @brief Event created when the window position has changed.
	*/
	class WindowPositionEvent : public Event
	{
		public:
			/*
			* @brief Creates an event announcing the window has moved.
			*
			* @param posX Coordinate X of the window position.
			* @param posX Coordinate X of the window position.
			*/
			WindowPositionEvent(int posX, int posY);

			virtual EventType getType() const override;

			/*
			* @brief Getter for the X coordinate of the window top left corner.
			*
			* @return Value of the X coordinate of the window position, in pixels.
			* The coordinates origin is located at the top left corner.
			*/
			int getPosX() const;
			/*
			* @brief Getter for the Y coordinate of the window top left corner.
			*
			* @return Value of the Y coordinate of the window position, in pixels.
			* The coordinates origin is located at the top left corner.
			*/
			int getPosY() const;

		private:
			int posX;
			int posY;
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
	* @brief Event class for adding or removing instances of a specific mesh on dispatch.
	*/
	class ChangeInstanceCountEvent : public Event
	{
		public:
			/*
			* @brief Creates an event to add or remove instances for a certain mesh.
			*
			* @param modelID String identifying the target model.
			* @param instanceCountVariation Quantity of instances to be added (positive value),
			* or removed (negative value).
			*/
			ChangeInstanceCountEvent(const char* modelID, int32_t instanceCountVariation);

			virtual EventType getType() const override;

			/*
			* @brief Getter for the target model ID.
			*
			* @return Identifier for the model which will have the mesh instance count altered.
			*/
			const std::string& getModelID() const;
			/*
			* @brief Getter for the variation of instance count.
			*
			* @return Amount of instances to be added or removed.
			*/
			int32_t getInstanceCountVariation() const;

		private:
			std::string modelID;
			int32_t instanceCountVariation;
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
