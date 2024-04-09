#pragma once

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
};
