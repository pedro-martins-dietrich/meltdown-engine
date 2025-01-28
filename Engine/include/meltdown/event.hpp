#pragma once

#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <type_traits>

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
	* @brief Container class to handle the event callback function after being registered in the `EventManager`.
	* The event callback tied to an instance of this class will be removed with the deletion of the instance.
	*/
	class EventCallbackHandle
	{
		public:
			EventCallbackHandle();
			EventCallbackHandle(std::type_index eventType, uint64_t callbackID);
			~EventCallbackHandle();

			EventCallbackHandle(const EventCallbackHandle&) = delete;
			EventCallbackHandle& operator=(const EventCallbackHandle&) = delete;

			EventCallbackHandle(EventCallbackHandle&& other) noexcept;
			EventCallbackHandle& operator=(EventCallbackHandle&& other) noexcept;

			/*
			* @brief Removes the callback tied to this object from the `EventManager`.
			*/
			void removeCallback();

		private:
			std::type_index eventType;
			uint64_t callbackID;
	};

	/*
	* @brief Base template struct for lambda function traits.
	*/
	template<typename T>
	struct LambdaTraits;
	/*
	* @brief Template struct containing type data for a lambda function.
	*/
	template<typename ReturnType, typename ClassType, typename... Args>
	struct LambdaTraits<ReturnType(ClassType::*)(Args...) const>
	{
		using ArgsTuple = std::tuple<Args...>;
		using Return = ReturnType;

		static constexpr size_t argCount = sizeof...(Args);
	};
	/*
	* @brief Helper template struct to check if the type overloads the `()` operator.
	*/
	template<typename T>
	using LambdaTraitsHelper = LambdaTraits<decltype(&T::operator())>;
	/*
	* @brief Template struct to verify the types used in a lambda function.
	*/
	template<typename T>
	struct LambdaTraits : LambdaTraitsHelper<T> {};

	/*
	* @brief Type for callback functions that accept a const reference to an `Event` as parameter
	* and have a `void` return type.
	*/
	using EventCallback = std::function<void(const Event&)>;

	/*
	* @brief Handles the event dispatches and callbacks of the engine.
	*/
	namespace EventManager
	{
		/*
		* @brief Adds a callback function to the specified event type.
		*
		* @param eventType Index of the event type associated with the callback.
		* @param callback Lambda function called when the event is triggered.
		* The callback must receive a `const Event&` argument and return `void`.
		*
		* @return Object to handle the callback's lifetime.
		*/
		EventCallbackHandle addCallback(std::type_index eventType, const EventCallback& callback);
		/*
		* @brief Adds a callback function to the event type specified in the lambda function parameter.
		*
		* @param callback Lambda function called when the event is triggered.
		* The callback must receive a `const EventType&` argument and return `void`.
		*
		* @return Object to handle the callback's lifetime.
		*/
		template<typename Callback>
		EventCallbackHandle addCallback(const Callback& callback)
		{
			static_assert(std::is_void_v<typename LambdaTraits<Callback>::Return>, "Callback must have a void return type.");
			static_assert(LambdaTraits<Callback>::argCount == 1, "Callback must take exactly 1 argument.");

			using Arguments = typename LambdaTraits<Callback>::ArgsTuple;
			using EventType = typename std::remove_cvref_t<std::tuple_element_t<0, Arguments>>;
			static_assert(std::is_base_of_v<Event, EventType>, "Callback parameter must be derived from the Event class.");

			return addCallback(std::type_index{typeid(EventType)}, [callback](const Event& event)
			{
				callback(static_cast<const EventType&>(event));
			});
		}

		/*
		* @brief Dispatches an event to be handled.
		*
		* @param pEvent Unique pointer to an event, transfering the pointer ownership to the `EventManager`.
		*/
		void dispatch(std::unique_ptr<Event> pEvent);
		/*
		* @brief Dispatches an event to be handled.
		*
		* @param args Arguments to construct the event instance.
		*/
		template<typename EventType, typename... Args>
		void dispatch(Args&&... args)
		{
			static_assert(std::is_base_of_v<Event, EventType>, "Callback parameter must be derived from the Event class.");
			static_assert(std::is_constructible_v<EventType, Args...>, "Arguments does not match the event type constructor.");

			dispatch(std::make_unique<EventType>(std::forward<Args>(args)...));
		}
	};
}
