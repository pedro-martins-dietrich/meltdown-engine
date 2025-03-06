#pragma once

#include "../Frame/Swapchain.hpp"
#include "../Frame/Framebuffer.hpp"
#include "../ImGui/ImGuiHandler.hpp"
#include "../../Scene/Scene.hpp"

namespace mtd
{
	// Responsible for rendering the current scene to the screen
	class Renderer
	{
		public:
			Renderer();
			~Renderer() = default;

			Renderer(const Renderer&) = delete;
			Renderer& operator=(const Renderer&) = delete;

			// Getter
			std::vector<RenderPassInfo>& getRenderOrder() { return renderOrder; }

			// Setter
			void setClearColor(const Vec4& color);

			// Renders frame to screen
			void render
			(
				const Device& device,
				const Swapchain& swapchain,
				const ImGuiHandler& guiHandler,
				const std::vector<Framebuffer>& framebuffers,
				const std::vector<GraphicsPipeline>& graphicsPipelines,
				const std::vector<FramebufferPipeline>& framebufferPipelines,
				const std::vector<RayTracingPipeline>& rayTracingPipelines,
				const Scene& scene,
				DrawInfo& drawInfo,
				bool& shouldUpdateEngine
			);

		private:
			// Index of the frame being rendered
			uint32_t currentFrameIndex;
			// Framebuffer clear color
			vk::ClearColorValue clearColor;
			// Order which the framebuffers will be rendered
			std::vector<RenderPassInfo> renderOrder;

			// Records draw commands to the command buffer
			void recordDrawCommands
			(
				const std::vector<Framebuffer>& framebuffers,
				const std::vector<GraphicsPipeline>& graphicsPipelines,
				const std::vector<FramebufferPipeline>& framebufferPipelines,
				const std::vector<RayTracingPipeline>& rayTracingPipelines,
				const Scene& scene,
				const CommandHandler& commandHandler,
				const DrawInfo& drawInfo,
				const ImGuiHandler& guiHandler,
				const vk::detail::DispatchLoaderDynamic& dldi
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
