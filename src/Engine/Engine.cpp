#include "Engine.hpp"

mtd::Engine::Engine()
	: window{FrameDimensions{800, 600}},
	vulkanInstance{"Meltdown", VK_MAKE_API_VERSION(0, 1, 0, 0), window},
	device{vulkanInstance},
	swapchain{device, window.getDimensions(), vulkanInstance.getSurface()}
{
}

// Begins the engine main loop
void mtd::Engine::start()
{
	while(window.keepOpen())
	{
	}
}
