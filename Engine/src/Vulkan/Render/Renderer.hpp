#pragma once

#include "RenderObjectManager.hpp"
#include "../Frame/Swapchain.hpp"
#include "../Frame/Framebuffer.hpp"
#include "../ImGui/ImGuiHandler.hpp"
#include "../Pipeline/PipelineBundles.hpp"
#include "../../Scene/Scene.hpp"

namespace mtd
{
	// Responsible for rendering the current scene to the screen
	class Renderer
	{
		public:
			Renderer(const Device& mtdDevice);
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
				const Swapchain& swapchain,
				const ImGuiHandler& guiHandler,
				const std::vector<Framebuffer>& framebuffers,
				const PipelineBundle& pipelines,
				const Scene& scene,
				ResourceManager& resourceManager,
				DescriptorSetHandler& globalDescriptorSet,
				DrawInfo& drawInfo,
				std::atomic<bool>& shouldUpdateEngine
			);

			// Configures the descriptor for the render objects
			void configureRendererDescriptor
			(
				ResourceManager& resourceManager, DescriptorSetHandler& descriptorSetHandler
			);

		private:
			// Index of the frame being rendered
			uint32_t currentFrameIndex = 0U;
			// Framebuffer clear values
			std::array<vk::ClearValue, 2> clearValues;
			// Order which the framebuffers will be rendered
			std::vector<RenderPassInfo> renderOrder;

			// Handler for per frame data to be sent to the GPU
			RenderObjectManager renderObjectManager;

			// Device reference
			const Device& mtdDevice;

			// Records draw commands to the command buffer
			void recordDrawCommands
			(
				const std::vector<Framebuffer>& framebuffers,
				const PipelineBundle& pipelines,
				const Scene& scene,
				const ResourceManager& resourceManager,
				const CommandHandler& commandHandler,
				const DrawInfo& drawInfo,
				const std::vector<DrawBatch>& drawBatches,
				const ImGuiHandler& guiHandler
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
