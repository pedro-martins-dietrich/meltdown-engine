#include <pch.hpp>
#include "SettingsGui.hpp"

static const std::array<const char*, 6> presentModeNames =
{
	"Immediate",
	"Mailbox",
	"FIFO",
	"FIFO relaxed",
	"Shared demand refresh",
	"Shared continuous refresh"
};

mtd::SettingsGui::SettingsGui(SwapchainSettings& swapchainSettings, bool& shouldUpdateEngine)
	: GuiWindow{ImVec2{450.0f, 300.0f}, ImVec2{20.0f, 120.0f}},
	swapchainSettings{swapchainSettings}, shouldUpdateEngine{shouldUpdateEngine}
{
}

// Exhibits the GUI window
void mtd::SettingsGui::renderGui()
{
	if(!showWindow) return;

	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGui::Begin("Engine Settings", &showWindow);

	swapchainSettingsGui();

	ImGui::End();
}

// Swapchain settings section
void mtd::SettingsGui::swapchainSettingsGui()
{
	ImGui::SeparatorText("Swapchain");

	ImGui::Text("Swapchain frame count:");
	int frameCount = static_cast<int>(swapchainSettings.frameCount);
	shouldUpdateEngine |= ImGui::InputInt("##Frame count", &frameCount, 1, 1);
	swapchainSettings.frameCount = static_cast<uint32_t>(frameCount);

	ImGui::Text("Composite alpha:");
	uint32_t compositeAlphaNumber = static_cast<uint32_t>(swapchainSettings.compositeAlpha);
	shouldUpdateEngine |= ImGui::CheckboxFlags
	(
		"Opaque",
		&compositeAlphaNumber,
		static_cast<uint32_t>(vk::CompositeAlphaFlagBitsKHR::eOpaque)
	);
	shouldUpdateEngine |= ImGui::CheckboxFlags
	(
		"Pre-multiplied",
		&compositeAlphaNumber,
		static_cast<uint32_t>(vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
	);
	shouldUpdateEngine |= ImGui::CheckboxFlags
	(
		"Post-multiplied",
		&compositeAlphaNumber,
		static_cast<uint32_t>(vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
	);
	shouldUpdateEngine |= ImGui::CheckboxFlags
	(
		"Inherit",
		&compositeAlphaNumber,
		static_cast<uint32_t>(vk::CompositeAlphaFlagBitsKHR::eInherit)
	);
	swapchainSettings.compositeAlpha =
		static_cast<vk::CompositeAlphaFlagBitsKHR>(compositeAlphaNumber);

	ImGui::Separator();

	ImGui::Text("Present mode:");
	int presentModeIndex =
		(swapchainSettings.presentMode == vk::PresentModeKHR::eSharedDemandRefresh) ? 4
		: (swapchainSettings.presentMode == vk::PresentModeKHR::eSharedContinuousRefresh) ? 5
		: static_cast<int>(swapchainSettings.presentMode);
	shouldUpdateEngine |= ImGui::Combo
	(
		"##PresentMode", &presentModeIndex, presentModeNames.data(), presentModeNames.size()
	);
	swapchainSettings.presentMode =
		(presentModeIndex == 4) ? vk::PresentModeKHR::eSharedDemandRefresh
		: (presentModeIndex == 5) ? vk::PresentModeKHR::eSharedContinuousRefresh
		: static_cast<vk::PresentModeKHR>(presentModeIndex);
}
