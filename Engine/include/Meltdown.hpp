#pragma once

#include <memory>

#include <meltdown/structs.hpp>

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
	* Call run() to start the engine.
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
}
