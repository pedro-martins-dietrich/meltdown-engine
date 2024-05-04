#include "SettingsGui.hpp"

#include <imgui.h>

mtd::SettingsGui::SettingsGui
(
	SwapchainSettings& swapchainSettings,
	PipelineSettings& pipelineSettings,
	bool& shouldUpdateEngine
) : swapchainSettings{swapchainSettings},
	pipelineSettings{pipelineSettings},
	shouldUpdateEngine{shouldUpdateEngine},
	showGui{true}
{
	setNames();
}

// Exhibits the GUI window
void mtd::SettingsGui::renderGui()
{
	ImGui::SetNextWindowSize(ImVec2{450.0f, 300.0f});
	ImGui::SetNextWindowPos(ImVec2{20.0f, 120.0f});
	ImGui::Begin("Engine Settings", &showGui);

	swapchainSettingsGui();
	pipelineSettingsGui();

	ImGui::End();
}

// Swapchain settings section
void mtd::SettingsGui::swapchainSettingsGui()
{
	ImGui::SeparatorText("Swapchain");

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

// Pipeline settings section
void mtd::SettingsGui::pipelineSettingsGui()
{
	ImGui::SeparatorText("Pipeline");

	ImGui::Text("Input assembly primitive topology:");
	int topologyIndex = static_cast<int>(pipelineSettings.inputAssemblyPrimitiveTopology);
	shouldUpdateEngine |= ImGui::Combo
	(
		"##Primitive topology",
		&topologyIndex,
		primitiveTopologyNames.data(),
		primitiveTopologyNames.size()
	);
	pipelineSettings.inputAssemblyPrimitiveTopology =
		static_cast<vk::PrimitiveTopology>(topologyIndex);

	ImGui::Separator();

	ImGui::Text("Rasterizer polygon mode:");
	int polygonModeIndex =
		(pipelineSettings.rasterizationPolygonMode == vk::PolygonMode::eFillRectangleNV) ? 3
		: static_cast<int>(pipelineSettings.rasterizationPolygonMode);
	shouldUpdateEngine |= ImGui::Combo
	(
		"##Polygon mode", &polygonModeIndex, polygonModeNames.data(), polygonModeNames.size()
	);
	pipelineSettings.rasterizationPolygonMode = (polygonModeIndex == 3)
		? vk::PolygonMode::eFillRectangleNV
		: static_cast<vk::PolygonMode>(polygonModeIndex);

	ImGui::Separator();

	ImGui::Text("Rasterizer cull mode:");
	int cullModeIndex = static_cast<uint32_t>(pipelineSettings.rasterizationCullMode);
	shouldUpdateEngine |=
		ImGui::Combo("##Cull mode", &cullModeIndex, cullModeNames.data(), cullModeNames.size());
	pipelineSettings.rasterizationCullMode = static_cast<vk::CullModeFlags>(cullModeIndex);

	ImGui::Separator();

	ImGui::Text("Rasterizer front face:");
	int frontFace = static_cast<int>(pipelineSettings.rasterizationFrontFace);
	shouldUpdateEngine |= ImGui::RadioButton("Counter-clockwise", &frontFace, 0);
	shouldUpdateEngine |= ImGui::RadioButton("Clockwise", &frontFace, 1);
	pipelineSettings.rasterizationFrontFace = static_cast<vk::FrontFace>(frontFace);
}

// Configures strings to show in the GUI
void mtd::SettingsGui::setNames()
{
	presentModeNames =
	{
		"Immediate",
		"Mailbox",
		"FIFO",
		"FIFO relaxed",
		"Shared demand refresh",
		"Shared continuous refresh"
	};

	primitiveTopologyNames =
	{
		"Point list",
		"Line list",
		"Line strip",
		"Triangle list",
		"Triangle strip",
		"Triangle fan",
		"Line list with adjacency",
		"Line strip with adjacency",
		"Triangle list with adjacency",
		"Triangle strip with adjacency",
		"Patch list"
	};

	polygonModeNames =
	{
		"Fill",
		"Line",
		"Point",
		"Fill rectangle NV"
	};

	cullModeNames =
	{
		"None",
		"Front",
		"Back",
		"Front and back"
	};
}
