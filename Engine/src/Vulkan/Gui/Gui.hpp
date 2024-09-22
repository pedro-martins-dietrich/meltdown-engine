#pragma once

#include "../Descriptors/DescriptorPool.hpp"
#include "GuiWindow.hpp"

namespace mtd
{
	// Renders a Graphical User Interface (GUI)
	class Gui
	{
		public:
			Gui(const vk::Device& vulkanDevice);
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

			// Adds a GUI window to exhibit
			void addGuiWindow(GuiWindow* pGuiWindow) { guiWindows.push_back(pGuiWindow); }

			// Renders the GUI in the current frame
			void renderGui(const vk::CommandBuffer& commandBuffer) const;

		private:
			// ImGui descriptor pool
			DescriptorPool guiDescriptorPool;

			// Exhibit GUI
			bool showGui;

			std::vector<GuiWindow*> guiWindows;

			// Checks ImGui Vulkan results
			static void checkVulkanResult(VkResult result);

			// Creates the descriptor pool for ImGui
			void createDescriptorPool();
			// Configures the input logic for toggling the GUI
			void setInputCallbacks();
	};
}
