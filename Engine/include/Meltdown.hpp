#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <meltdown/enums.hpp>
#include <meltdown/structs.hpp>
#include <meltdown/math.hpp>

#ifndef MTD_SHARED
	#define MELTDOWN_API
#elif defined(_WIN32)
	#ifdef MTD_EXPORTS
		#define MELTDOWN_API __declspec(dllexport)
	#else
		#define MELTDOWN_API __declspec(dllimport)
	#endif
#elif defined(__linux__) || defined(__APPLE__)
	#ifdef MTD_EXPORTS
		#define MELTDOWN_API __attribute__((visibility("default")))
	#else
		#define MELTDOWN_API
	#endif
#else
	#define MELTDOWN_API
#endif

/*
* @brief Namespace used by the Meltdown (mtd) Engine.
*/
namespace mtd
{
	class Engine;

	/*
	* @brief Meltdown Engine main class.
	* Load a scene with `loadScene()` and then call `run()` to start the engine.
	*/
	class MELTDOWN_API MeltdownEngine
	{
		public:
			/*
			* @brief Initializes the application window, Vulkan and other engine dependencies.
			*
			* @param applicationInfo Information about the application name and version.
			*/
			MeltdownEngine(const EngineInfo& applicationInfo);
			~MeltdownEngine();

			MeltdownEngine(const MeltdownEngine&) = delete;
			MeltdownEngine& operator=(const MeltdownEngine&) = delete;

			/*
			* @brief Sets a new value for the clear color of the framebuffer.
			*
			* @param color The RGBA value for the clear color
			* (each channel should be between 0.0f and 1.0f).
			*/
			void setClearColor(const Vec4& color);
			/*
			* @brief Enables or disables the Vertical Synchronization (V-Sync).
			* By default, the engine starts with V-Sync enabled.
			*
			* @param enableVSync Value to which the V-Sync will be set.
			*/
			void setVSync(bool enableVSync);

			/*
			* @brief Begins the engine main loop, transfering the application control to the engine.
			*
			* @param onUpdateCallback Callback function called on every engine update.
			*/
			void run(const std::function<void(double)>& onUpdateCallback);

			/*
			* @brief Loads a new scene to be rendered by the engine. It needs to be called before
			* starting the main engine loop.
			* If a scene is already being rendered, it will be cleared before loading the new scene.
			*
			* @param sceneFile Path to the file containing the new scene data.
			*/
			void loadScene(const char* sceneFile);

		private:
			std::unique_ptr<Engine> engine;
	};

	/*
	* @brief Set of functions to control the camera.
	*/
	namespace CameraHandler
	{
		/*
		* @brief Getter for the camera position.
		*
		* @return Coordinates of the camera.
		*/
		const Vec3& MELTDOWN_API getPosition();
		/*
		* @brief Getter for the camera view direction.
		*
		* @return Normalized ray direction of the camera view.
		*/
		Vec3 MELTDOWN_API getViewDirection();
		/*
		* @brief Getter for the camera right direction vector, in its local coordinate system.
		*
		* @return Right direction of the camera's local coordinate system.
		*/
		Vec3 MELTDOWN_API getRightDirection();
		/*
		* @brief Getter for the camera up direction vector, in its local coordinate system.
		*
		* @return Up direction of the camera's local coordinate system.
		*/
		Vec3 MELTDOWN_API getUpDirection();

		/*
		* @brief Getter for the perspective camera FOV.
		*
		* @return Vertical FOV of the camera, in degrees.
		*/
		float MELTDOWN_API getFOV();
		/*
		* @brief Getter for the perspective camera near plane.
		*
		* @return Distance from the camera position to the nearest region rendered.
		*/
		float MELTDOWN_API getNearPlane();
		/*
		* @brief Getter for the camera far plane.
		*
		* @return Distance from the camera position to the farthest region rendered.
		*/
		float MELTDOWN_API getFarPlane();
		/*
		* @brief Getter for the orthographic camera view width.
		*
		* @return Horizontal width of the orthographic camera view.
		*/
		float MELTDOWN_API getViewWidth();

		/*
		* @brief Checks if the camera is in perspective or orthographic mode.
		*
		* @return `true` when in orthographic mode, or `false` in perspective mode.
		*/
		bool MELTDOWN_API isOrthographic();

		/*
		* @brief Sets the camera position to the specified location.
		*
		* @param newPosition Coordinates of the new camera position.
		*/
		void MELTDOWN_API setPosition(const Vec3& newPosition);
		/*
		* @brief Sets the camera view direction to the specified orientation.
		*
		* @param yaw Angle of rotation around the world Y axis, in radians [0, 2*PI].
		* @param pitch Angle of rotation around the camera's X axis, in radians [-PI/2, PI/2].
		*/
		void MELTDOWN_API setOrientation(float yaw, float pitch, float roll = 0.0f);
		/*
		* @brief Sets the camera view direction to the specified orientation.
		*
		* @param newOrientation Quaternion describing the camera orientation.
		* The quaternion should be normalized to avoid weird camera behavior.
		*/
		void MELTDOWN_API setOrientation(const Quaternion& newOrientation);

		/*
		* @brief Moves the camera from the current location by the specified amount.
		*
		* @param deltaPos Translation amount for each coordinate.
		*/
		void MELTDOWN_API translate(const Vec3& deltaPos);
		/*
		* @brief Rotates the camera view direction by the specified amount.
		*
		* @param deltaYaw Rotation amount, in radians, around the world Y axis.
		* @param deltaPitch Rotation amount, in radians, around the camera X axis.
		*/
		void MELTDOWN_API rotate(float deltaYaw, float deltaPitch, float deltaRoll = 0.0f);
		/*
		* @brief Rotates the camera view direction by the specified amount.
		*
		* @param quaternion Rotation in world coordinates, described as a quaternion.
		*/
		void MELTDOWN_API rotate(const Quaternion& quaternion);
	};

	/*
	* @brief Handles the key mapping to actions in the engine.
	*/
	namespace Input
	{
		/*
		* @brief Creates an action, mapping a set of keys that need to be pressed to trigger
		* that action. If a mapping already exists for the action, it will be overwritten.
		* When the keys are pressed, an `ActionStartEvent` is dispatched, and when released,
		* an `ActionStopEvent` gets dispatched.
		*
		* @param action An enum to which a set of keys is assigned, present in the action events.
		* @param keyCodes A vector of key codes that maps to the physical keys of the keyboard.
		* All keys in the list must be pressed at the same time to start the action.
		*/
		void MELTDOWN_API mapAction(uint32_t action, std::vector<KeyCode>&& keyCodes);
		/*
		* @brief Removes the action from the engine, ceasing the dispatch of the action events.
		*
		* @param action The enumeration/ID to the action to be removed.
		*/
		void MELTDOWN_API unmapAction(uint32_t action);
	}
}
