#pragma once

#include "../Pipeline/Pipeline.hpp"

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
				const Device& device,
				const Swapchain& swapchain,
				const Gui& gui,
				const std::unordered_map<PipelineType, Pipeline>& pipelines,
				DrawInfo& drawInfo,
				bool& shouldUpdateEngine
			);

		private:
			// Index of the frame being rendered
			uint32_t currentFrameIndex;

			// Records draw command to the command buffer
			void recordDrawCommand
			(
				const std::unordered_map<PipelineType, Pipeline>& pipelines,
				const CommandHandler& commandHandler,
				const DrawInfo& drawInfo,
				const Gui& gui
			) const;

			// Presents frame to screen when ready
			void presentFrame
			(
				const vk::SwapchainKHR& swapchain,
				const vk::Queue& presentQueue,
				const vk::Semaphore& renderFinished
			) const;
	};
}
