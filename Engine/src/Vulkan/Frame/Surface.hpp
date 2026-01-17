#pragma once

#include "../../Window/WindowHandler.hpp"

namespace mtd
{
	// Handles the Vulkan surface of the window
	class Surface
	{
		public:
			Surface(const vk::Instance& instance, const WindowHandler* pWindowHandler);
			~Surface();

			Surface(const Surface&) = delete;
			Surface& operator=(const Surface&) = delete;

			// Getter
			const vk::SurfaceKHR& getSurface() const { return surface; }

		private:
			// Vulkan surface
			vk::SurfaceKHR surface;

			// Vulkan instance reference
			const vk::Instance& instance;
	};
}
