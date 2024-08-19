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
			*/
			MeltdownEngine(const EngineInfo& applicationInfo);
			~MeltdownEngine();

			MeltdownEngine(const MeltdownEngine&) = delete;
			MeltdownEngine& operator=(const MeltdownEngine&) = delete;

			/*
			* @brief Begins the engine main loop, transfering the application control to the engine.
			*/
			void run();

		private:
			std::unique_ptr<Engine> engine;
	};
}
