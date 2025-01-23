#include <pch.hpp>
#include "SettingsGui.hpp"

#include <meltdown/event.hpp>

static const std::array<const char*, 6> presentModeNames =
{
	"Immediate",
	"Mailbox",
	"FIFO",
	"FIFO relaxed",
	"Shared demand refresh",
	"Shared continuous refresh"
};

mtd::SettingsGui::SettingsGui(SwapchainSettings& swapchainSettings, const Camera& camera, bool& shouldUpdateEngine)
	: GuiWindow{ImVec2{450.0f, 300.0f}, ImVec2{20.0f, 120.0f}},
	swapchainSettings{swapchainSettings}, shouldUpdateEngine{shouldUpdateEngine},
	orthographicMode{camera.isOrthographic()}, nearPlane{camera.getNearPlane()}, farPlane{camera.getFarPlane()},
	yFOV{camera.getFOV()}, viewWidth{camera.getViewWidth()}, updateCamera{false}
{
	EventManager::addCallback(EventType::SetPerspectiveCamera, [this](const Event& e)
	{
		const SetPerspectiveCameraEvent* spce = dynamic_cast<const SetPerspectiveCameraEvent*>(&e);
		if(!spce) return;

		orthographicMode = false;
		yFOV = spce->getFOV();
		nearPlane = spce->getNearPlane();
		farPlane = spce->getFarPlane();
	});

	EventManager::addCallback(EventType::SetOrthographicCamera, [this](const Event& e)
	{
		const SetOrthographicCameraEvent* soce = dynamic_cast<const SetOrthographicCameraEvent*>(&e);
		if(!soce) return;

		orthographicMode = true;
		viewWidth = soce->getViewWidth();
		farPlane = soce->getFarPlane();
	});
}

// Exhibits the GUI window
void mtd::SettingsGui::renderGui()
{
	if(!showWindow) return;

	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGui::Begin("Engine Settings", &showWindow);

	swapchainSettingsGui();
	cameraSettingsGui();

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
	swapchainSettings.compositeAlpha = static_cast<vk::CompositeAlphaFlagBitsKHR>(compositeAlphaNumber);

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

// Camera settings section
void mtd::SettingsGui::cameraSettingsGui()
{
	ImGui::SeparatorText("Camera");

	ImGui::Text("Mode:");
	if(ImGui::RadioButton("Perspective", !orthographicMode) && orthographicMode)
	{
		orthographicMode = false;
		updateCamera = true;
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Orthographic", orthographicMode) && !orthographicMode)
	{
		orthographicMode = true;
		updateCamera = true;
	}

	ImGui::Separator();

	updateCamera |= ImGui::InputFloat("Far plane", &farPlane, 0.1f, 1.0f, "%.1f");

	if(orthographicMode)
		updateCamera |= ImGui::InputFloat("View width", &viewWidth, 0.01f, 1.0f, "%.2f");
	else
	{
		updateCamera |= ImGui::InputFloat("Near plane", &nearPlane, 0.001f, 0.01f, "%.3f");
		updateCamera |= ImGui::DragFloat("FOV", &yFOV, 0.1f, 0.1f, 175.0f, "%.1f");
	}

	if(updateCamera)
	{
		updateCamera = false;
		if(orthographicMode)
			EventManager::dispatch(std::make_unique<SetOrthographicCameraEvent>(viewWidth, farPlane));
		else
			EventManager::dispatch(std::make_unique<SetPerspectiveCameraEvent>(yFOV, nearPlane, farPlane));
	}
}
