#pragma once

#include <functional>
#include <memory>
#include <string>

#include <meltdown/enums.hpp>
#include <meltdown/math.hpp>

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
	* @brief Event created when a mouse movement is detected.
	*/
	class MousePositionEvent : public Event
	{
		public:
			/*
			* @brief Creates an event indicating the mouse position has changed.
			*
			* @param xPos Horizontal coordinate of the mouse, relative to the window center.
			* The value ranges between `-aspectRatio` and `+aspectRatio`, when inside the window.
			* @param yPos Vertical coordinate of the mouse, relative to the window center.
			* The value ranges between `-1` and `1`, when inside the window.
			*/
			MousePositionEvent(float xPos, float yPos, bool cursorHidden);

			virtual EventType getType() const override;

			/*
			* @brief Getter for the mouse position coordinates, relative to the screen center.
			*
			* @return Mouse cursor coordinates, ranging between `-1` and `1` vertically,
			* and `-aspectRatio` and `+aspectRatio` horizontally.
			*/
			const Vec2& getMousePosition() const;

			/*
			* @brief Hidden status of the cursor.
			* When hidden, the cursor resets its position to the window center every frame.
			*
			* @return Equals `true` when cursor is hidden, or `false` when visible.
			*/
			bool isCursorHidden() const;

		private:
			Vec2 position;
			bool cursorHidden;
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
	* @brief Event to change the perspective camera configuration.
	* If the camera is in the orthographic mode, it will change to perspective.
	*/
	class SetPerspectiveCameraEvent : public Event
	{
		public:
			/*
			* @brief Creates an event to configure the camera to use a perspective view with the provided data.
			*
			* @param yFOV Angle, in degrees, of the camera's field of view (FOV), for the vertical axis.
			* @param nearPlane Distance to the closest region from the camera to get rendered.
			* @param farPlane Maximum render distance.
			*/
			SetPerspectiveCameraEvent(float yFOV, float nearPlane, float farPlane);

			virtual EventType getType() const override;

			/*
			* @brief Getter for the camera's field of view (FOV).
			*
			* @return Value, in degrees, for the vertical field of view (FOV) of the camera.
			*/
			float getFOV() const;
			/*
			* @brief Getter for the camera's near plane distance.
			*
			* @return View frustrum nearest plane distance.
			*/
			float getNearPlane() const;
			/*
			* @brief Getter for the camera's far plane distance.
			*
			* @return View frustrum farthest plane distance.
			*/
			float getFarPlane() const;

		private:
			float yFOV;
			float nearPlane;
			float farPlane;
	};

	/*
	* @brief Event to change the orthographic camera configuration.
	* If the camera is in the perspective mode, it will change to orthographic.
	*/
	class SetOrthographicCameraEvent : public Event
	{
		public:
			/*
			* @brief Creates an event to configure the camera to use a orthographic view with the provided data.
			*
			* @param viewWidth Width of the orthographic camera view.
			* The view height will be set according to the window's aspect ratio.
			* @param farPlane Maximum render distance from the camera.
			*/
			SetOrthographicCameraEvent(float viewWidth, float farPlane);

			virtual EventType getType() const override;

			/*
			* @brief Getter for the camera's view width.
			*
			* @return Value for the orthographic camera view width.
			*/
			float getViewWidth() const;
			/*
			* @brief Getter for the camera's far plane distance. The near plane always intersects the camera position.
			*
			* @return View frustrum farthest plane distance.
			*/
			float getFarPlane() const;

		private:
			float viewWidth;
			float farPlane;
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
	* @brief Event class for adding instances of a specific mesh.
	*/
	class CreateInstancesEvent : public Event
	{
		public:
			/*
			* @brief Creates an event to add instances for a certain model.
			*
			* @param modelID String identifying the target model.
			* @param instanceCount Quantity of instances to be added.
			*/
			CreateInstancesEvent(const char* modelID, uint32_t instanceCount);

			virtual EventType getType() const override;

			/*
			* @brief Getter for the target model ID.
			*
			* @return Identifier for the model which will have the mesh instance count increased.
			*/
			const std::string& getModelID() const;
			/*
			* @brief Getter for the instance creation count.
			*
			* @return Amount of instances to be added.
			*/
			uint32_t getInstanceCount() const;

		private:
			std::string modelID;
			uint32_t instanceCount;
	};

	/*
	* @brief Event class for remove a model instance from the current scene.
	*/
	class RemoveInstanceEvent : public Event
	{
		public:
			/*
			* @brief Creates an event to delete an instance from the scene.
			*
			* @param instanceID Identifier for the target model instance to be removed.
			*/
			RemoveInstanceEvent(uint64_t instanceID);

			virtual EventType getType() const override;

			/*
			* @brief Getter for the target instance ID.
			*
			* @return Identifier for the model instance which will deleted from the current scene.
			*/
			uint64_t getInstanceID() const;

		private:
			uint64_t instanceID;
	};

	/*
	* @brief Event for updating the descriptor content at the specified location.
	*/
	class UpdateDescriptorDataEvent : public Event
	{
		public:
			/*
			* @brief Creates an event to update a descriptor data.
			*
			* @param pipelineIndex Index, in the current scene, for the target pipeline.
			* @param binding Descriptor binding index for the target descriptor resource.
			* @param data Pointer to the updated descriptor data.
			*/
			UpdateDescriptorDataEvent(uint32_t pipelineIndex, uint32_t binding, const void* data);

			virtual EventType getType() const override;

			/*
			* @brief Getter for the target pipeline index.
			*
			* @return Index for the pipeline in the current scene.
			*/
			uint32_t getPipelineIndex() const;

			/*
			* @brief Getter for the target descriptor binding index.
			*
			* @return Index for the descriptor binding.
			*/
			uint32_t getBinding() const;

			/*
			* @brief Getter for the new descriptor data.
			*
			* @return Pointer to the data that will be copied to the descriptor.
			*/
			const void* getData() const;

		private:
			uint32_t pipelineIndex;
			uint32_t binding;
			const void* data;
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
	};
}
