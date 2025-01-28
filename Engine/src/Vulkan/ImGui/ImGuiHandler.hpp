#pragma once

#include "../Descriptors/DescriptorPool.hpp"
#include "../../GUIs/GuiWindow.hpp"

namespace mtd
{
	// Renders a Graphical User Interface (GUI) using ImGui
	class ImGuiHandler
	{
		public:
			ImGuiHandler(const vk::Device& vulkanDevice);
			~ImGuiHandler();

			ImGuiHandler(const ImGuiHandler&) = delete;
			ImGuiHandler& operator=(const ImGuiHandler&) = delete;

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
			// List of windows to be rendered
			std::vector<GuiWindow*> guiWindows;

			// Toggle GUI event callback
			EventCallbackHandle toggleGuiCallbackHandle;

			// Checks ImGui Vulkan results
			static void checkVulkanResult(VkResult result);

			// Creates the descriptor pool for ImGui
			void createDescriptorPool();
			// Configures the input logic for toggling the GUI
			void setInputCallbacks();
	};
}
