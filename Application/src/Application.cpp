#include "Application.hpp"

#include <meltdown/structs.hpp>

Application::Application()
	: meltdownEngine{mtd::EngineInfo{"Meltdown Application", 1, 0, 0}}
{
}

// Begins the engine main loop
void Application::run()
{
	meltdownEngine.run();
}
