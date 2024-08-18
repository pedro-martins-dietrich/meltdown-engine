#include "Application.hpp"

Application::Application() : meltdownEngine{}
{
}

// Begins the engine main loop
void Application::run()
{
	meltdownEngine.run();
}
