#pragma once

#include "../Descriptors/DescriptorPool.hpp"

namespace mtd
{
	// Renders a Graphical User Interface (GUI)
	class Gui
	{
		public:
			Gui(const vk::Device& vulkanDevice, InputHandler& inputHandler);
			~Gui();

			Gui(const Gui&) = delete;
			Gui& operator=(const Gui&) = delete;

			// Initializes ImGui
			void init
			(
				const Window& window,
				const vk::Instance& instance,
				const Device& device,
				const vk::RenderPass& renderPass,
				uint32_t framesInFlight
			) const;

			// Renders the GUI in the current frame
			void renderGui(const vk::CommandBuffer& commandBuffer) const;

		private:
			// ImGui descriptor pool
			DescriptorPool guiDescriptorPool;

			// Exhibit GUI
			bool showGui;

			// Checks ImGui Vulkan results
			static void checkVulkanResult(VkResult result);

			// Creates the descriptor pool for ImGui
			void createDescriptorPool();
			// Configures the input logic for the GUI
			void setInputCallbacks(InputHandler& inputHandler);
	};
}
