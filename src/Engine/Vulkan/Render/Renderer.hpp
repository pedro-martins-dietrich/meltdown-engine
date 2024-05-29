#pragma once

#include "../Frame/Swapchain.hpp"

namespace mtd
{
	// Responsible for rendering the current scene to the screen
	class Renderer
	{
		public:
			Renderer();
			~Renderer() {}

			Renderer(const Renderer&) = delete;
			Renderer& operator=(const Renderer&) = delete;

			// Renders frame to screen
			void render
			(
				const vk::Device& device,
				const Swapchain& swapchain,
				const Gui& gui,
				DrawInfo& drawInfo,
				bool& shouldUpdateEngine
			);

		private:
			// Index of the frame being rendered
			uint32_t currentFrameIndex;
	};
}
