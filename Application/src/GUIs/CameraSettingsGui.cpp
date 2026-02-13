#include "CameraSettingsGui.hpp"

#include <imgui.h>
#include <Meltdown.hpp>

CameraSettingsGui::CameraSettingsGui()
	: windowSize{450.0f, 300.0f}, windowPos{20.0f, 120.0f},
	orthographicMode{mtd::CameraHandler::isOrthographic()},
	nearPlane{mtd::CameraHandler::getNearPlane()}, farPlane{mtd::CameraHandler::getFarPlane()},
	yFOV{mtd::CameraHandler::getFOV()}, viewWidth{mtd::CameraHandler::getViewWidth()}, updateCamera{false}
{
	mtd::EventManager::addCallback([this](const mtd::SetPerspectiveCameraEvent& event)
	{
		orthographicMode = false;
		yFOV = event.getFOV();
		nearPlane = event.getNearPlane();
		farPlane = event.getFarPlane();
	});

	mtd::EventManager::addCallback([this](const mtd::SetOrthographicCameraEvent& event)
	{
		orthographicMode = true;
		viewWidth = event.getViewWidth();
		farPlane = event.getFarPlane();
	});
}

void CameraSettingsGui::renderGui()
{
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGui::Begin("Engine Settings", &showWindow);

	cameraSettingsGui();

	ImGui::End();
}

void CameraSettingsGui::cameraSettingsGui()
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
			mtd::EventManager::dispatch<mtd::SetOrthographicCameraEvent>(viewWidth, farPlane);
		else
			mtd::EventManager::dispatch<mtd::SetPerspectiveCameraEvent>(yFOV, nearPlane, farPlane);
	}
}
