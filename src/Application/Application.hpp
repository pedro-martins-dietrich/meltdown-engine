#pragma once

#include "../Engine/Engine.hpp"

// Main class responsible to interact with the engine
class Application
{
	public:
		Application();
		~Application() {}

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		// Begins the engine main loop
		void run();

	private:
		// Instance of the Meltdown Engine
		mtd::Engine meltdownEngine;
};
