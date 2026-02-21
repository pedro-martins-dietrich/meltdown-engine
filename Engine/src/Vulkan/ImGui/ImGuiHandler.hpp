#pragma once

#include <meltdown/gui.hpp>

#include "../Descriptors/DescriptorPool.hpp"

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
				const vk::Instance& instance,
				const Device& device,
				const vk::RenderPass& renderPass,
				uint32_t framesInFlight
			);

			// Adds a GUI window to exhibit
			void addGuiWindow(GuiWindow* const pGuiWindow) { guiWindows.push_back(pGuiWindow); }

			// Renders the GUI in the current frame
			void renderGui(const vk::CommandBuffer& commandBuffer) const;

		private:
			// ImGui descriptor pool
			DescriptorPool guiDescriptorPool;

			// Flag to check if ImGui has been initialized
			bool initializedFlag;
			// List of windows to be rendered
			std::vector<GuiWindow*> guiWindows;

			// Checks ImGui Vulkan results
			static void checkVulkanResult(VkResult result);

			// Creates the descriptor pool for ImGui
			void createDescriptorPool();
	};
}
