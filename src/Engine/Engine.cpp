#include "Engine.hpp"

mtd::Engine::Engine() : window{800, 600}
{
}

// Begins the engine main loop
void mtd::Engine::start()
{
	while(window.keepOpen())
	{
	}
}
