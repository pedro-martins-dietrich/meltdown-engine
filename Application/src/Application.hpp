#pragma once

#include <Meltdown.hpp>

// Main class responsible to interact with the engine
class Application
{
	public:
		Application();
		~Application() = default;

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		// Begins the engine main loop
		void run();

	private:
		// Instance of the Meltdown Engine
		mtd::MeltdownEngine meltdownEngine;
};
