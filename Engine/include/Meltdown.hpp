#pragma once

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
			* @brief Begins the engine main loop, transfering the application control to the engine.
			*/
			void run();

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
